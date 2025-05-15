#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

#include "imu/imu.h"
#include "log/log.h"
#include "rc/rc.h"
#include "sys/sys.h"
#include "uart/uart.h"

#define DSP368_PRS_CFG_REG 0x06
#define DSP368_TMP_CFG_REG 0x07
#define DSP368_MEAS_CFG_REG 0x08
#define DSP368_PRS_DATA_REG 0x00
#define DSP368_TMP_DATA_REG 0x03

static imu_Imu _sys_ImuInstance;
static rc_Rc _sys_RcInstance;

static bool _spi_WriteBlocking(uint8_t regAddress, uint8_t data) {
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

static bool _spi_ReadBlocking(uint8_t regAddress, uint8_t numBytes, volatile uint8_t* buffer) {
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

void init_hardware() {
    uart_Init(&sys_UartInstance, (uart_UartInitParams) //
              { .huart = &huart1,
                .uartIr = USART1_IRQn,
                .txBufferLength = 256,
                .rxBufferLength = 256,
                .ignorableChars = "\r",
                .endOfMsgChar = '\n' });

    tel_Init();
    log_Init();

    log_Debug("Initalizing hardware...");

    rc_Init(&_sys_RcInstance, &huart5);

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
    log_Info("RC test mode");

    rc_RxPackage rcData = { 0 };
    while (true) {
        if (rc_GetData(&_sys_RcInstance, &rcData)) {
            for (int i = 0; i < 18; i++)
                log_Raw("%d,", rcData.channels[i]);
        } else
            log_Error("Failed to get RC data");

        log_Raw("\r\n");
        HAL_Delay(50);
    }

#elif defined(BARO_ESC_TEST)
    log_Info("Barometer and ESC test mode");

    uint8_t tmp[6] = { 0 };

    HAL_Delay(1000);

    _spi_ReadBlocking(DSP368_MEAS_CFG_REG, 1, tmp);
    const char* response = (tmp[0] & 0xc0) == 0xc0 ? "Hello, successful world!\r\n" : "Hello, failed world!\r\n";
    log_Info(response);
    HAL_Delay(10);

    _spi_WriteBlocking(DSP368_MEAS_CFG_REG, 0x07);
    HAL_Delay(10);

    _spi_WriteBlocking(DSP368_PRS_CFG_REG, 0x36);
    HAL_Delay(10);

    _spi_WriteBlocking(DSP368_TMP_CFG_REG, 0xa0);
    HAL_Delay(10);

    char rxBuf[100];
    uint32_t rxSize = 0;

    bool log = true;

    while (1) {
        _spi_ReadBlocking(DSP368_PRS_DATA_REG, 6, tmp);

        int32_t pressure = (tmp[0] << 16) | (tmp[1] << 8) | tmp[2];
        if (pressure & 0x800000) {
            pressure |= 0xff000000;
        }

        int32_t temperature = (tmp[3] << 16) | (tmp[4] << 8) | tmp[5];
        if (temperature & 0x800000) {
            temperature |= 0xff000000;
        }

        if (log)
            log_Raw("%ld,%ld\r\n", pressure, temperature);

        uart_ReceiveStatus status = uart_Receive(&sys_UartInstance, rxBuf + rxSize, sizeof(rxBuf) - 1 - rxSize);
        rxSize += status.size;

        if (status.eomReached) {
            rxBuf[rxSize] = '\0';

            log_Info("Received: %s\r\n", rxBuf);

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
    log_Info("IMU test mode");
    log_Debug("Initializing hardware...");

    uint8_t initSuccess = imu_Init(&_sys_ImuInstance, &hspi2, IMU_CS_GPIO_Port, IMU_CS_Pin, SPI2_IRQn, &htim9);
    imu_SetDefaultSettings(&_sys_ImuInstance);

    log_Debug("Initialization %s", initSuccess ? "successful" : "failed");
#endif
}

void init_modules() {
    log_Debug("Initializing software modules...");
}

void init() {
    log_Debug("Initializing...");

    init_hardware();
    HAL_Delay(10);
    init_modules();
}

void sys_Entry(void) {
    init();

    while (1) {
        imu_Vec3 acc = imu_ReadAccData(&_sys_ImuInstance);
        imu_Vec3 gyro = imu_ReadGyroData(&_sys_ImuInstance);
        float temp = imu_ReadTempData(&_sys_ImuInstance);

        log_Raw("%lf,%lf,%lf,%lf,%lf,%lf,%lf\r\n", acc.x, acc.y, acc.z, gyro.x, gyro.y, gyro.z, temp);

        HAL_Delay(50);
    }
}
