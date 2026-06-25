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
#include "main.h"
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

static imu_Imu _sys_imuInstance;

static TIM_HandleTypeDef* const _sys_usTimer = &htim2;
static TIM_HandleTypeDef* const _sys_imuTimer = &htim10;

static TIM_HandleTypeDef* const _sys_actTimer = &htim3;

static ADC_HandleTypeDef* const _sys_batteryADC = &hadc1;

static UART_HandleTypeDef* const _sys_rcUsart = &huart5;
static UART_HandleTypeDef* const _sys_telUart = &huart1;

static SPI_HandleTypeDef* const _sys_imuSpi = &hspi2;

static void _sys_init_drivers(void) {
    log_debug("Initalizing drivers...");

    act_init(_sys_actTimer);

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

    log_debug("Init done");
    HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
}

typedef struct _sys_Telemetry {
    ctrl_Mode ctrlMode;
    llc_ThrustVec guideRef;
    llc_ThrustVec llcOut;
    act_FinalSignalTelemetry actOut;
} _sys_Telemetry;

static _sys_Telemetry _sys_lastTel = { 0 };
static void _sys_doTelemetry(sys_TimeUS now) {
    const uint32_t telemetryPeriodUS = 1e6 / config_TELEMETRY_FREQ;
    static sys_TimeUS lastTelemetry = 0;
    static sys_TimeUS lastKnownArmChange = 0;

    if (now < lastTelemetry + telemetryPeriodUS)
        return;

    bool isArmed = act_isArmed();
    sys_TimeUS armChangeT = act_getArmTime();
    if (isArmed)
        armChangeT = act_getArmTime();
    else
        armChangeT = act_getDisarmTime();

    bool armingChanged = lastKnownArmChange != armChangeT || (lastTelemetry == 0 && lastKnownArmChange == 0);
    if (armingChanged) {
        lastKnownArmChange = armChangeT;
    }

    lastTelemetry = now;

    // ctrl_Mode ctrl_mode = _sys_lastTel.ctrlMode;
    llc_ThrustVec guideRef = _sys_lastTel.guideRef;
    llc_ThrustVec llcOut = _sys_lastTel.llcOut;
    act_FinalSignalTelemetry actOut = _sys_lastTel.actOut;

    if (isArmed) {
        if (armingChanged) {
            log_raw(
                "# vbat,imu_roll,imu_pitch,imu_yaw,"
                "llc_roll,llc_pitch,llc_yaw,llc_thrust," //
                "motor0,motor1,motor2,motor3"            //
            );
        }

        imu_Vec3 v3 = dsp_getOutAng();
        log_raw(
            "%.2f,"
            "%.2f,%.2f,%.2f,"
            // "%.2f,%.2f,%.2f,%.2f,",
            "%u,%u,%u,%u",
            (double) adc_getBatteryVoltage() / config_BATTERY_CELL_COUNT,                                         //
            (double) utils_radToDeg(v3.roll), (double) utils_radToDeg(v3.pitch), (double) utils_radToDeg(v3.yaw), //
        // (double) utils_radToDeg(llcOut.roll), (double) utils_radToDeg(llcOut.pitch),                          //
        // (double) utils_radToDeg(llcOut.yaw), (double) llcOut.thrust,                                          //

#if config_ACT_MODE == config_ACT_MODE_DSHOT
            actOut.dshotThrottles[0], //
            actOut.dshotThrottles[1], //
            actOut.dshotThrottles[2], //
            actOut.dshotThrottles[3]  //
#elif config_ACT_MODE == config_ACT_MODE_PWM
            actOut.pwmDuties[0], //
            actOut.pwmDuties[1], //
            actOut.pwmDuties[2], //
            actOut.pwmDuties[3]  //
#endif
        );
    } else {
        if (armingChanged) {
            log_raw(
                "# vbat,imu_roll,imu_pitch,imu_yaw," //
                "rc_roll,rc_pitch,rc_yaw,rc_thrust"  //
            );
        }

        imu_Vec3 v3 = dsp_getOutAng();
        log_raw(
            "%.2f,"
            "%.2f,%.2f,%.2f,"
            "%.2f,%.2f,%.2f,%.2f",
            (double) adc_getBatteryVoltage() / config_BATTERY_CELL_COUNT,                                         //
            (double) utils_radToDeg(v3.roll), (double) utils_radToDeg(v3.pitch), (double) utils_radToDeg(v3.yaw), //
            (double) utils_radToDeg(guideRef.roll), (double) utils_radToDeg(guideRef.pitch),                      //
            (double) utils_radToDeg(guideRef.yaw), (double) guideRef.thrust                                       //
        );
    }
}

static void _sys_guide(void) {
    sys_TimeUS lastRCFrameTime = sys_rcInstance.lastFrameTime;
    sys_TimeUS now = sys_time();

    ctrl_Mode ctrlMode = ctrl_getMode();

    // TODO: use one rc channel to select what to print

    // disarm if in rc mode and no sbus data has arrived in a while
    sys_TimeUS lastRc = sys_timeDiff(lastRCFrameTime, now);
    if (ctrlMode == ctrl_RC && act_isArmed() && lastRc > config_RC_IDLE_DISARM_MS * 1000) {
        log_warn("last RC signal was %" PRIu32 "ms ago, disarming", lastRc);
        act_disarm();
    }

    llc_ThrustVec guideRef = guide_getRef(ctrlMode);
    _sys_lastTel.ctrlMode = ctrlMode;
    _sys_lastTel.guideRef = guideRef;

    if (act_isArmed()) {
        llc_ThrustVec llcOut = llc_update(guideRef);
        act_FinalSignalTelemetry actOut = act_output(llcOut);

        _sys_lastTel.llcOut = llcOut;
        _sys_lastTel.actOut = actOut;
    }

    _sys_doTelemetry(now);
}

bool _sys_shouldLoop(void) {
    // TODO: more sophisticated
    // return true;

    static sys_TimeUS batteryDippedMinT = 0;

    sys_TimeUS now = sys_time();

    float voltage = adc_getBatteryVoltage();
    if (voltage < config_MIN_BATTERY_CELL_VOLTAGE * config_BATTERY_CELL_COUNT) {
        if (batteryDippedMinT == 0)
            batteryDippedMinT = now;
        else if (sys_timeDiff(batteryDippedMinT, now) > config_BATTERY_CRITICAL_TIME_MS * 1000)
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
    sys_TimeUS guideLoopLengthUS = 1 * 1000 * 1000 / config_ACT_LOOP_FREQ;

    // TODO: use more precise timer
    sys_TimeUS nextGuide = sys_time() + guideLoopLengthUS;
    while (_sys_shouldLoop()) {
        sys_TimeUS now = sys_time();

        if (_sys_shouldDoDsp())
            _sys_doDsp();

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

typedef uint32_t sys_TimeUS;

sys_TimeUS sys_time(void) {
    return _sys_usTimer->Instance->CNT;
}

uint64_t sys_totalTimeSinceBootUS(void) {
    return sys_time() + UINT32_MAX * _sys_bootTimerOverflows;
}

sys_TimeUS sys_timeDiff(sys_TimeUS earlier, sys_TimeUS later) {
    if (earlier <= later)
        return later - earlier;

    // there was an overlow between the two, earlier was before overflow
    return (UINT32_MAX - earlier) + later;
}

void sys_busyWait(sys_TimeUS t) {
    sys_busyWaitUntil(sys_time() + t);
}

void sys_busyWaitUntil(sys_TimeUS until) {
    if (sys_time() < until) {
        while (sys_time() < until) {
            /* __NOP(); */
        }
    } else {
        while (sys_time() > until) {
            /* __NOP(); */
        }
    }
}
