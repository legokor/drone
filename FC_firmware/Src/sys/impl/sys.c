#include "sys/sys.h"
#include "act/act.h"
#include "bar/bar.h"
#include "imu/imu.h"
#include "log/log.h"
#include "rc/rc.h"
#include "tel/tel.h"

#include "stm32f4xx_hal.h"

#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DSP368_PRS_CFG_REG 0x06
#define DSP368_TMP_CFG_REG 0x07
#define DSP368_MEAS_CFG_REG 0x08
#define DSP368_PRS_DATA_REG 0x00
#define DSP368_TMP_DATA_REG 0x03

uart_Uart sys_uartInstance;
static imu_Imu _sys_imuInstance;
static rc_Rc _sys_rcInstance;

static bool _sys_initalized = false;

static bool _spi_writeBlocking(uint8_t regAddress, uint8_t data) {
    uint8_t txData[] = { regAddress, data };
    bool ok = false;

    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 0);

    while (true) {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi2, txData, 2, 10);

        if (status != HAL_BUSY) {
            ok = status == HAL_OK;
            break;
        }
    }

    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 1);

    return ok;
}

static bool _spi_readBlocking(uint8_t regAddress, uint8_t numBytes, volatile uint8_t* buffer) {
    regAddress |= 0x80;
    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 0);

    bool ok = false;
    while (true) {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi2, &regAddress, 1, 10);

        if (status != HAL_BUSY) {
            ok = status == HAL_OK;
            break;
        }
    }

    if (ok) {
        ok = HAL_SPI_Receive(&hspi2, (uint8_t*) buffer, numBytes, 10 * numBytes) == HAL_OK;
    }

    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 1);

    return ok;
}

static void _sys_init_hardware() {
    uart_init(&sys_uartInstance, (uart_UartInitParams) //
              { .huart = &huart1,
                .uartIrq = USART1_IRQn,
                .txBufferLength = 256,
                .rxBufferLength = 256,
                .ignorableChars = "\r",
                .endOfMsgChar = '\n' });

    tel_init();
    log_init();

    log_debug("Initalizing hardware...");

    rc_init(&_sys_rcInstance, &huart5);

    HAL_TIM_Base_Start_IT(&htim9);
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, 1);
    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 1);

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    htim3.Instance->CCR1 = 1000;
    htim3.Instance->CCR2 = 1000;
    htim3.Instance->CCR3 = 1000;
    htim3.Instance->CCR4 = 1000;

// #define BARO_ESC_TEST
#define RC_TEST

#ifdef RC_TEST
    log_info("RC test mode");

    rc_RxPackage rcData = { 0 };
    while (true) {
        if (rc_getData(&_sys_rcInstance, &rcData)) {
            for (int i = 0; i < 18; i++)
                log_raw("%d,", rcData.channels[i]);
        } else
            log_error("Failed to get RC data");

        log_raw("\r\n");
        HAL_Delay(50);
    }

#elif defined(BARO_ESC_TEST)
    log_info("Barometer and ESC test mode");

    uint8_t tmp[6] = { 0 };

    HAL_Delay(1000);

    _spi_readBlocking(DSP368_MEAS_CFG_REG, 1, tmp);
    const char* response = (tmp[0] & 0xc0) == 0xc0 ? "Hello, successful world!\r\n" : "Hello, failed world!\r\n";
    log_info(response);
    HAL_Delay(10);

    _spi_writeBlocking(DSP368_MEAS_CFG_REG, 0x07);
    HAL_Delay(10);

    _spi_writeBlocking(DSP368_PRS_CFG_REG, 0x36);
    HAL_Delay(10);

    _spi_writeBlocking(DSP368_TMP_CFG_REG, 0xa0);
    HAL_Delay(10);

    char rxBuf[100];
    uint32_t rxSize = 0;

    bool log = true;

    while (1) {
        _spi_readBlocking(DSP368_PRS_DATA_REG, 6, tmp);

        int32_t pressure = (tmp[0] << 16) | (tmp[1] << 8) | tmp[2];
        if (pressure & 0x800000) {
            pressure |= 0xff000000;
        }

        int32_t temperature = (tmp[3] << 16) | (tmp[4] << 8) | tmp[5];
        if (temperature & 0x800000) {
            temperature |= 0xff000000;
        }

        if (log) {
            int alt = bar_calculateAltitude(pressure, temperature);
            log_raw("%ld,%ld,%ld\r\n", pressure, temperature, alt);
        }

        uart_ReceiveStatus status = uart_receive(&sys_uartInstance, rxBuf + rxSize, sizeof(rxBuf) - 1 - rxSize);
        rxSize += status.size;

        if (status.eomReached) {
            rxBuf[rxSize] = '\0';

            log_info("Received: %s\r\n", rxBuf);

            int motorId = 0;
            int motorSpeed = 0;
            if (sscanf(rxBuf, "M%d %d", &motorId, &motorSpeed) == 2) {
                if (motorSpeed < 1000)
                    motorSpeed = 1000;
                else if (motorSpeed > 2000)
                    motorSpeed = 2000;

                switch (motorId) {
                    case 1: htim3.Instance->CCR1 = motorSpeed; break;
                    case 2: htim3.Instance->CCR2 = motorSpeed; break;
                    case 3: htim3.Instance->CCR3 = motorSpeed; break;
                    case 4: htim3.Instance->CCR4 = motorSpeed; break;
                    default: break;
                }
            } else if (strcmp(rxBuf, "STOP") == 0) {
                htim3.Instance->CCR1 = 1000;
                htim3.Instance->CCR2 = 1000;
                htim3.Instance->CCR3 = 1000;
                htim3.Instance->CCR4 = 1000;
            } else if (strcmp(rxBuf, "L") == 0) {
                log = !log;
            }

            rxSize = 0;
        } else if (status.size > 0) {
            if (rxSize >= sizeof(rxBuf) - 1)
                rxSize = 0;
        }

        HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
        HAL_Delay(50);
    }
#else
    log_info("IMU test mode");
    log_debug("Initializing hardware...");

    uint8_t initSuccess = imu_init(&_sys_imuInstance, &hspi2, IMU_CS_GPIO_Port, IMU_CS_Pin, SPI2_IRQn, &htim9);
    imu_setDefaultSettings(&_sys_imuInstance);

    log_debug("Initialization %s", initSuccess ? "successful" : "failed");
#endif
}

static void _sys_init_modules(void) {
    log_debug("Initializing software modules...");
}

static void _sys_init(void) {
    log_debug("Initializing...");

    _sys_init_hardware();
    HAL_Delay(10);
    _sys_init_modules();

    _sys_initalized = true;
}

void sys_entry(void) {
    _sys_init();

    while (1) {
        imu_Vec3 acc = imu_readAccData(&_sys_imuInstance);
        imu_Vec3 gyro = imu_readGyroData(&_sys_imuInstance);
        float temp = imu_readTempData(&_sys_imuInstance);

        log_raw("%lf,%lf,%lf,%lf,%lf,%lf,%lf\r\n", acc.x, acc.y, acc.z, gyro.x, gyro.y, gyro.z, temp);

        HAL_Delay(50);
    }
}

bool sys_initalized(void) {
    return _sys_initalized;
}

void sys_abort(sys_AbortFn fn, void* arg) {
    act_disarm();

    fn(arg);

    while (true) {
    }
}
