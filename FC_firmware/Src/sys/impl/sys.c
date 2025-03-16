#include "sys/sys.h"
#include <stdio.h>
#include <string.h>
#include "log/log.h"
#include "main.h"
#include "spi.h"
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "usart.h"

sys_System sys_Instance;

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
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, 0);
    HAL_Delay(100);
    uint8_t initSuccess = imu_Init(&sys_Instance.imu, &hspi2, IMU_CS_GPIO_Port, IMU_CS_Pin, SPI2_IRQn, &htim9);
    imu_SetDefaultSettings(&sys_Instance.imu);

    {
        uart_UartInitParams uartInitParams = { .huart = &huart1,
                                               .uartIr = USART1_IRQn,
                                               .txBufferLength = 256,
                                               .rxBufferLength = 256,
                                               .ignorableChars = "\r",
                                               .endOfMsgChar = '\n' };
        uart_Init(&sys_Instance.uart, uartInitParams);
    }

    const char* response = initSuccess ? "Hello, successful world!\r\n" : "Hello, failed world!\r\n";
    uart_Transmit(&sys_Instance.uart, response, strlen(response));

    while (1) {
        imu_Vec3 acc = imu_ReadAccData(&sys_Instance.imu);
        imu_Vec3 gyro = imu_ReadGyroData(&sys_Instance.imu);
        float temp = imu_ReadTempData(&sys_Instance.imu);
        char buf[256];
        sprintf(buf, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\r\n", acc.x, acc.y, acc.z, gyro.x, gyro.y, gyro.z, temp);
        uart_Transmit(&sys_Instance.uart, buf, strlen(buf));
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
