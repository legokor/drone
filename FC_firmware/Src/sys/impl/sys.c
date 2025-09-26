#include "sys/sys.h"
#include "act/act.h"
#include "bar/bar.h"
#include "ctrl/ctrl.h"
#include "dsp/dsp.h"
#include "err/err.h"
#include "guide/guide.h"
#include "imu/imu.h"
#include "llc/llc.h"
#include "log/log.h"
#include "rc/rc.h"
#include "tel/tel.h"

#include "config.h"

#include "main.h"
#include "tim.h"
#include "usart.h"

#include "stm32f4xx_hal.h"

#include <stddef.h>
#include <stdint.h>

uart_Uart sys_uartInstance;
rc_Rc sys_rcInstance;

static imu_Imu _sys_imuInstance;

static bool _sys_initalized = false;

static void _sys_init_drivers(void) {
    log_debug("Initalizing drivers...");

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

    TIM_HandleTypeDef* ts[] = { &htim3, &htim3, &htim3, &htim3 };
    uint32_t chns[] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };
    act_init(ts, chns);

    ctrl_init();
    guide_init();
    llc_init();

    dsp_init();
}

static void _sys_writeUart(uint32_t t, tel_Topic topic, const void* data, size_t len, tel_DataType type) {
    // TODO: move + packetize
    err_tryIgnorable(uart_transmit(&sys_uartInstance, data, len), "failed to write through debug uart");
}

static void _sys_init(void) {
    // init debug uart
    (void) uart_init(      //
        &sys_uartInstance, //
        (uart_UartInitParams) {
            //
            .huart = &huart1,
            .uartIrq = USART1_IRQn,
            .txBufferLength = 2048,
            .rxBufferLength = 256,
            .ignorableChars = "\r",
            .endOfMsgChar = '\n' //
        });

    tel_init();
    tel_addSource(_sys_writeUart);
    log_init();

    log_debug("Initializing...");

    _sys_init_drivers();
    HAL_Delay(20);
    _sys_init_modules();

    err_tryFatal(imu_calculateGyroOffset(&_sys_imuInstance), "Couldn't calculate gyro offsets");
    imu_enableGyroOffsetSubtraction(&_sys_imuInstance, true);

    _sys_initalized = true;
}

void sys_entry(void) {
    _sys_init();

    int guideLoopLengthMS = 1000 / CONFIG_ACT_FREQ;

    // TODO: use more precise timer
    uint32_t nextGuide = HAL_GetTick() + guideLoopLengthMS;
    while (true) {
        imu_Vec3 acc;
        err_tryFatal(imu_readAccData(&_sys_imuInstance, &acc), "Failed to read IMU acc");
        dsp_setInAcc(acc);

        imu_Vec3 gyro;
        err_tryFatal(imu_readGyroData(&_sys_imuInstance, &gyro), "Failed to read IMU gyro");
        dsp_setInGyr(gyro);

        dsp_update();

        imu_Vec3 a = dsp_getOutAng();

        if (nextGuide <= HAL_GetTick()) {
            ctrl_Mode ctrl_mode = ctrl_getMode();
            llc_ThrustVec guide_ref = guide_getRef(ctrl_mode);
            llc_ThrustVec llc_out = llc_update(guide_ref);
            act_output(llc_out);

            nextGuide += guideLoopLengthMS;
            log_raw("%.2f,%.2f", (double) utils_RAD_TO_DEG(a.roll), (double) utils_RAD_TO_DEG(a.pitch));
        }
    }
}

bool sys_initalized(void) {
    return _sys_initalized;
}

void sys_abort(sys_AbortFn fn, void* arg) {
    act_disarm();

    if (fn != NULL) {
        fn(arg);
    }

    while (true) {
        __NOP();
    }
}
