#include "sys/sys.h"
#include "act/act.h"
#include "adc/adc.h"
#include "ctrl/ctrl.h"
#include "dsp/dsp.h"
#include "err/err.h"
#include "guide/guide.h"
#include "imu/imu.h"
#include "irq/irq.h"
#include "llc/llc.h"
#include "log/log.h"
#include "rc/rc.h"
#include "sys/time.h"
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

static uint32_t _sys_bootTimerOverflows = 0;

static imu_Imu _sys_imuInstance;

static bool _sys_isInitalized = false;
static bool _sys_wasArmed = false;

static imu_Imu _sys_imuInstance;

static TIM_HandleTypeDef* const _sys_usTimer = &htim2;
static TIM_HandleTypeDef* const _sys_imuTimer = &htim10;

static TIM_HandleTypeDef* const _sys_actTimers[] = { &htim3, &htim3, &htim3, &htim3 };
static const uint32_t _sys_actTimersChannels[] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };

static ADC_HandleTypeDef* const _sys_batteryADC = &hadc1;

static UART_HandleTypeDef* const _sys_rcUsart = &huart5;
static UART_HandleTypeDef* const _sys_telUart = &huart1;

static SPI_HandleTypeDef* const _sys_imuSpi = &hspi2;

static void _sys_init_drivers(void) {
    log_debug("Initalizing drivers...");

    act_init(_sys_actTimers, _sys_actTimersChannels);

    adc_init(_sys_batteryADC);

    err_tryFatal(rc_init(&sys_rcInstance, _sys_rcUsart), "Failed to init rc");

    HAL_TIM_Base_Start_IT(_sys_imuTimer);
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, 1);

    err_tryFatal(imu_init(&_sys_imuInstance, _sys_imuSpi, IMU_CS_GPIO_Port, IMU_CS_Pin, SPI2_IRQn, _sys_imuTimer),
                 "Couldn't init imu");

    err_tryFatal(imu_setDefaultSettings(&_sys_imuInstance), "Couldn't set imu default params");

    HAL_Delay(1);

    err_tryFatal(imu_calculateGyroOffset(&_sys_imuInstance), "Couldn't calculate gyro offsets");
    imu_enableGyroOffsetSubtraction(&_sys_imuInstance, true);

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

static void _sys_timPeriodElapsedCallback(void* context) {
    (void) context;

    _sys_bootTimerOverflows++;
}

static void _sys_init(void) {

    HAL_TIM_Base_Start_IT(_sys_usTimer);
    irq_subscribeToIrq(irq_TIM_PERIOD_ELAPSED, _sys_timPeriodElapsedCallback, NULL, _sys_usTimer);

    uart_UartInitParams uartParams = (uart_UartInitParams){
        .huart = _sys_telUart,
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

    ctrl_setMode(ctrl_RC);

    _sys_isInitalized = true;
    _sys_wasArmed = act_isArmed();

    log_debug("Init done");
}

static void _sys_guide(void) {
    ctrl_Mode ctrl_mode = ctrl_getMode();
    llc_ThrustVec guide_ref = guide_getRef(ctrl_mode);

    // TODO: use one rc channel to select what to print

    // TODO:disarm if in rc mode and no sbus data has arrived in a while
    sys_TimeStampUS lastRc = sys_time() - sys_rcInstance.lastFrameTime;
    if (ctrl_mode == ctrl_RC && act_isArmed() && lastRc > config_RC_IDLE_DISARM_MS * 1000) {
        log_warn("last RC signal was " PRIu32 "ms ago, disarming", lastRc);
        act_disarm();
    }

    if (act_isArmed()) {
        llc_ThrustVec llc_out = llc_update(guide_ref);

        act_FinalSignalTelemetry act_out = act_output(llc_out);

        if (!_sys_wasArmed) {
            log_raw("# vbat,imu_roll,imu_pitch,imu_yaw"
                    // "llc_roll,llc_pitch,llc_yaw,llc_thrust" //
            );
        }

        imu_Vec3 v3 = dsp_getOutAng();
        log_raw(
            "%.2f,"
            "%.2f,%.2f,%.2f,",
            // "%.2f,%.2f,%.2f,%.2f",
            (double) adc_getBatteryVoltage() / config_BATTERY_CELL_COUNT,                                        //
            (double) utils_radToDeg(v3.roll), (double) utils_radToDeg(v3.pitch), (double) utils_radToDeg(v3.yaw) //
            // (double) utils_radToDeg(llc_out.roll), (double) utils_radToDeg(llc_out.pitch),                        //
            // (double) utils_radToDeg(llc_out.yaw), (double) llc_out.thrust                                         //
        );
    } else {
        if (_sys_wasArmed) {
            log_raw(
                "# vbat,imu_roll,imu_pitch,imu_yaw," //
                "rc_roll,rc_pitch,rc_yaw,rc_thrust"  //
            );
        }

        imu_Vec3 v3 = dsp_getOutAng();
        log_raw(
            "%.2f,"
            "%.2f,%.2f,%.2f,",
            // "%.2f,%.2f,%.2f,%.2f",
            (double) adc_getBatteryVoltage() / config_BATTERY_CELL_COUNT,                                        //
            (double) utils_radToDeg(v3.roll), (double) utils_radToDeg(v3.pitch), (double) utils_radToDeg(v3.yaw) //
            // (double) utils_radToDeg(guide_ref.roll), (double) utils_radToDeg(guide_ref.pitch),                    //
            // (double) utils_radToDeg(guide_ref.yaw), (double) guide_ref.thrust                                     //
        );
    }

    // {
    //     imu_Vec3 a = dsp_getInAcc();
    //     imu_Vec3 g = dsp_getInGyr();

    //     imu_Vec3 af = dsp_getOutFilteredAcc();
    //     imu_Vec3 gf = dsp_getOutFilteredGyr();

    //     log_raw(
    //         "%.2f,%.2f,%.2f,"
    //         "%.2f,%.2f,%.2f,"
    //         "%.2f,%.2f,%.2f,"
    //         "%.2f,%.2f,%.2f",
    //         (double) a.x, (double) a.y, (double) a.z,          //
    //         (double) g.roll, (double) g.pitch, (double) g.yaw, //
    //         (double) af.x, (double) af.y, (double) af.z,       //
    //         (double) gf.roll, (double) gf.pitch, (double) gf.yaw);
    // }
    _sys_wasArmed = act_isArmed();
}

bool _sys_shouldLoop(void) {
    // TODO: more sophisticated
    return true;

    static sys_TimeStampUS batteryDippedMinT = 0;

    float voltage = adc_getBatteryVoltage();
    if (voltage < config_MIN_BATTERY_CELL_VOLTAGE * config_BATTERY_CELL_COUNT) {
        if (batteryDippedMinT == 0)
            batteryDippedMinT = sys_time();
        else if (sys_time() - batteryDippedMinT > config_BATTERY_CRITICAL_TIME_MS * 1000)
            return false;
    } else
        batteryDippedMinT = 0;

    return true;
}

static bool _sys_shouldDoDsp(void) {
    return imu_newDataAvailable(&_sys_imuInstance);
}

static void _sys_doDsp(void) {
    imu_Vec3 acc;
    err_tryFatal(imu_readAccData(&_sys_imuInstance, &acc), "Failed to read IMU acc");
    dsp_setInAcc(acc);

    imu_Vec3 gyro;
    err_tryFatal(imu_readGyroData(&_sys_imuInstance, &gyro), "Failed to read IMU gyro");
    dsp_setInGyr(gyro);

    dsp_update();
}

void _sys_loop(void) {
    sys_TimeStampUS guideLoopLengthUS = 1 * 1000 * 1000 / config_ACT_LOOP_FREQ;

    // TODO: use more precise timer
    sys_TimeStampUS nextGuide = sys_time() + guideLoopLengthUS;
    while (_sys_shouldLoop()) {
        sys_TimeStampUS now = sys_time();

        if (_sys_shouldDoDsp())
            _sys_doDsp();

        log_debug("time: %u", now);

        if (nextGuide <= now) {
            _sys_guide();
            nextGuide += guideLoopLengthUS;
        }
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

bool sys_isInitalized(void) {
    return _sys_isInitalized;
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

typedef uint32_t sys_TimeStampUS;

sys_TimeStampUS sys_time(void) {
    return _sys_usTimer->Instance->CNT;
}

uint64_t sys_totalTimeSinceBootUS(void) {
    return sys_time() + UINT32_MAX * _sys_bootTimerOverflows;
}

sys_TimeStampUS sys_timeDiff(sys_TimeStampUS earlier, sys_TimeStampUS later) {
    if (earlier < later) [[likely]]
        return later - earlier;

    // there was an overlow between the two, earlier was before overflow
    return (UINT32_MAX - earlier) + later;
}

void sys_sleepUntill(sys_TimeStampUS untill) {
    while (sys_time() < untill) {
        /* __NOP(); */
    }
}
