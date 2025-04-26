#include "sys/sys.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "log/log.h"
#include "main.h"
#include "spi.h"
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "usart.h"

sys_System sys_Instance;

#define DSP368_PRS_CFG_REG 0x06
#define DSP368_TMP_CFG_REG 0x07
#define DSP368_MEAS_CFG_REG 0x08
#define DSP368_PRS_DATA_REG 0x00
#define DSP368_TMP_DATA_REG 0x03

static uint8_t _spi_WriteBlocking(uint8_t regAddress, uint8_t data) {
    uint8_t txData[] = { regAddress, data };
    uint8_t ok = 0;
    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 0);
    while (1) {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi2, txData, 2, 10);
        if (status != HAL_BUSY) {
            ok = status == HAL_OK;
            break;
        }
    }
    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 1);
    return ok;
}

static uint8_t _spi_ReadBlocking(uint8_t regAddress, uint8_t numBytes, volatile uint8_t* buffer) {
    uint8_t ok = 0;
    regAddress |= 0x80;
    HAL_GPIO_WritePin(BAR_CS_GPIO_Port, BAR_CS_Pin, 0);
    while (1) {
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
    log_Debug("Initalizing hardware...");

    //    lora_Init(&sys_Instance.lora);
    //    sd_Init(&sys_Instance.sd);
    //    rc_Init(&sys_Instance.rc);
    //    gps_Init(&sys_Instance.gps);
    //    mag_Init(&sys_Instance.mag);
    //    bar_Init(&sys_Instance.bar);
    //    esc_Init(&sys_Instance.esc);
    //    acc_Init(&sys_Instance.acc);
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

    // uint8_t initSuccess = imu_Init(&sys_Instance.imu, &hspi2, IMU_CS_GPIO_Port, IMU_CS_Pin, SPI2_IRQn, &htim9);
    // imu_SetDefaultSettings(&sys_Instance.imu);

    {
        uart_UartInitParams uartInitParams = { .huart = &huart1,
                                               .uartIr = USART1_IRQn,
                                               .txBufferLength = 256,
                                               .rxBufferLength = 256,
                                               .ignorableChars = "\r",
                                               .endOfMsgChar = '\n' };
        uart_Init(&sys_Instance.uart, uartInitParams);
    }

    uint8_t tmp[6] = { 0 };

    HAL_Delay(1000);
    _spi_ReadBlocking(DSP368_MEAS_CFG_REG, 1, tmp);
    const char* response = (tmp[0] & 0xc0) == 0xc0 ? "Hello, successful world!\r\n" : "Hello, failed world!\r\n";
    uart_Transmit(&sys_Instance.uart, response, strlen(response));

    HAL_Delay(10);
    _spi_WriteBlocking(DSP368_MEAS_CFG_REG, 0x07);
    HAL_Delay(10);
    _spi_WriteBlocking(DSP368_PRS_CFG_REG, 0x36);
    HAL_Delay(10);
    _spi_WriteBlocking(DSP368_TMP_CFG_REG, 0xa0);
    HAL_Delay(10);

    char rxBuf[100];
    uint32_t rxSize = 0;

    bool log = false;

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

        char buf[200];
        sprintf(buf, "%ld,%ld\r\n", pressure, temperature);
        if (log)
            uart_Transmit(&sys_Instance.uart, buf, strlen(buf));

        uart_ReceiveStatus status = uart_Receive(&sys_Instance.uart, rxBuf + rxSize, sizeof(rxBuf) - 1 - rxSize);
        rxSize += status.size;
        if (status.eomReached) {
            rxBuf[rxSize] = '\0';
            sprintf(buf, "Received: %s\r\n", rxBuf);
            uart_Transmit(&sys_Instance.uart, buf, strlen(buf));
            int motorId = 0;
            int motorSpeed = 0;
            if (sscanf(rxBuf, "M%d %d", &motorId, &motorSpeed) == 2) {
                if (motorSpeed < 1000) {
                    motorSpeed = 1000;
                } else if (motorSpeed > 2000) {
                    motorSpeed = 2000;
                }
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
            if (rxSize >= sizeof(rxBuf) - 1) {
                rxSize = 0;
            }
        }

        HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
        HAL_Delay(50);
    }
}

void init_modules() {
    log_Debug("Initalizing software modules...");

    //    tel_Init();
    //    act_Init();
    //    llc_Init();
    //    guide_Init();
    //    ctrl_Init();
    //    dsp_Init();
}

void init() {
    log_Debug("Initalizing...");

    init_hardware();
    init_modules();
}

void sys_Entry(void) {
    init();
}
