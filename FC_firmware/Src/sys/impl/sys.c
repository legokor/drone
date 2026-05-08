#include "sys/sys.h"
#include "act/act.h"
#include "adc/adc.h"
#include "ctrl/ctrl.h"
#include "dsp/dsp.h"
#include "err/err.h"
#include "guide/guide.h"
#include "imu/imu.h"
#include "llc/llc.h"
#include "log/log.h"
#include "rc/rc.h"
#include "tel/tel.h"
#include "uart/uart.h"

#include "config.h"

#include "adc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

#include "stm32f4xx_hal.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

uart_Uart sys_uartInstance;
rc_Rc sys_rcInstance;

static imu_Imu _sys_imuInstance;

static bool _sys_initalized = false;
static bool _sys_wasArmed = false;

static void _sys_init_drivers(void) {
    log_debug("Initalizing drivers...");

    TIM_HandleTypeDef* ts[] = { &htim3, &htim3, &htim3, &htim3 };
    uint32_t chns[] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };
    act_init(ts, chns);

    adc_init(&hadc1);

    err_tryFatal(rc_init(&sys_rcInstance, &huart5), "Failed to init rc");

    HAL_TIM_Base_Start_IT(&htim9);
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, 1);

    err_tryFatal(imu_init(&_sys_imuInstance, &hspi2, IMU_CS_GPIO_Port, IMU_CS_Pin, SPI2_IRQn, &htim9),
                 "Couldn't init imu");

    err_tryFatal(imu_setDefaultSettings(&_sys_imuInstance), "Couldn't set imu default params");

    // bar_init();
    // esc_init();
    // gps_init();
    // lora_init();
    // mag_init();
    // sd_init();
    // usb_init();
}

static void _sys_init_modules(void) {
    log_debug("Initializing software modules...");

    ctrl_init();
    guide_init();
    llc_init();

    dsp_init();
}

static void _sys_writeUart(uint32_t t, tel_Topic topic, const void* data, size_t len, tel_DataType type) {
    (void) type;
    (void) t;

    // TODO: move + packetize
    if (topic != config_LOG_TOPIC)
        return;

    err_tryIgnorable(uart_transmit(&sys_uartInstance, data, len), "failed to write through debug uart");
}

static void _sys_init(void) {
    uart_UartInitParams uartParams = (uart_UartInitParams){
        .huart = &huart1,
        .uartIrq = USART1_IRQn,
        .txBufferLength = 1 << 14,
        .rxBufferLength = 1 << 10,
        .ignorableChars = "\r",
        .endOfMsgChar = '\n',
    };
    err_tryIgnorable(uart_init(&sys_uartInstance, uartParams), "failed to init uart");

    tel_init();
    tel_addSource(_sys_writeUart);
    log_init();

    log_debug("Initializing...");

    _sys_init_drivers();
    HAL_Delay(20);
    _sys_init_modules();

    err_tryFatal(imu_calculateGyroOffset(&_sys_imuInstance), "Couldn't calculate gyro offsets");
    imu_enableGyroOffsetSubtraction(&_sys_imuInstance, true);

    ctrl_setMode(ctrl_RC);

    _sys_initalized = true;
    _sys_wasArmed = act_isArmed();
}

static void _sys_guide(void) {
    ctrl_Mode ctrl_mode = ctrl_getMode();
    llc_ThrustVec guide_ref = guide_getRef(ctrl_mode);

    // TODO: use one rc channel to select what to print

    // TODO:disarm if in rc mode and no sbus data has arrived in a while
    uint32_t lastRc = HAL_GetTick() - sys_rcInstance.lastFrameTime;
    if (ctrl_mode == ctrl_RC && act_isArmed() && lastRc > config_NO_RC_DISARM_MS) {
        log_warn("last RC signal was " PRIu32 "ms ago, disarming", lastRc);
        act_disarm();
    }

    if (act_isArmed()) {
        llc_ThrustVec llc_out = llc_update(guide_ref);

        act_FinalSignalTelemetry act_out = act_output(llc_out);

        if (!_sys_wasArmed)
            log_raw("# imu_roll,imu_pitch,imu_yaw,rc_roll,rc_pitch,rc_yaw,rc_thrust,motor_0,motor_1,motor_2,motor_3");

        imu_Vec3 v3 = dsp_getOutAng();
        log_raw(
            "%.2f,%.2f,%.2f,"
            "%.2f,%.2f,%.2f,%.2f,"
            "%u,%u,%u,%u",
            (double) v3.roll, (double) v3.pitch, (double) v3.yaw, //
            (double) guide_ref.roll, (double) guide_ref.pitch,    //
            (double) guide_ref.yaw, (double) guide_ref.thrust,    //
            (unsigned int) act_out.motorSignals[0],               //
            (unsigned int) act_out.motorSignals[1],               //
            (unsigned int) act_out.motorSignals[2],               //
            (unsigned int) act_out.motorSignals[3]                //
        );
    } else {
        if (_sys_wasArmed)
            log_raw("# imu_roll,imu_pitch,imu_yaw,rc_roll,rc_pitch,rc_yaw,rc_thrust");

        imu_Vec3 v3 = dsp_getOutAng();
        log_raw(
            "%.2f,%.2f,%.2f,"
            "%.2f,%.2f,%.2f,%.2f",                                //
            (double) v3.roll, (double) v3.pitch, (double) v3.yaw, //
            (double) guide_ref.roll, (double) guide_ref.pitch,    //
            (double) guide_ref.yaw, (double) guide_ref.thrust     //
        );
    }
    _sys_wasArmed = act_isArmed();
}

bool _sys_shouldLoop(void) {
    // TODO: more sophisticated

    static uint32_t batteryDippedMin = 0;

    float voltage = adc_getBatteryVoltage();
    if (voltage < config_MIN_BATTERY_CELL_VOLTAGE * config_BATTERY_CELL_COUNT) {
        if (batteryDippedMin == 0)
            batteryDippedMin = HAL_GetTick();
        else if (HAL_GetTick() - batteryDippedMin > config_BATTERY_CRITICAL_TIME_MS)
            return false;
    } else
        batteryDippedMin = 0;

    return true;
}

void _sys_loop(void) {
    int guideLoopLengthMS = 1000 / config_ACT_FREQ;

    // TODO: use more precise timer
    uint32_t nextGuide = HAL_GetTick() + guideLoopLengthMS;
    while (_sys_shouldLoop()) {
        imu_Vec3 acc;
        err_tryFatal(imu_readAccData(&_sys_imuInstance, &acc), "Failed to read IMU acc");
        dsp_setInAcc(acc);

        imu_Vec3 gyro;
        err_tryFatal(imu_readGyroData(&_sys_imuInstance, &gyro), "Failed to read IMU gyro");
        dsp_setInGyr(gyro);

        dsp_update();
        if (nextGuide <= HAL_GetTick()) {
            _sys_guide();
            nextGuide += guideLoopLengthMS;
        } else
            // because dsp was way faster than sensors
            // TODO: something more sophisticated
            HAL_Delay(1);
    }

    if (act_isArmed())
        act_disarm();

    log_error("battery critical, disarmed");
}

void sys_entry(void) {
    _sys_init();
    _sys_loop();
    sys_abort(NULL, NULL);
}

bool sys_initalized(void) {
    return _sys_initalized;
}

void sys_abort(sys_AbortFn fn, void* arg) {
    if (act_isArmed())
        act_disarm();

    if (fn != NULL) {
        fn(arg);
    }

    while (true) {
        __NOP();
    }
}
