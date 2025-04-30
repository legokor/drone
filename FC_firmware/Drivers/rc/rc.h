#ifndef RC_H
#define RC_H

#include <stdbool.h>
#include "stm32f4xx_hal.h"

typedef struct rc_RxPackage {
    uint16_t channels[18];
    bool frameLost;
    bool failsafeActive;
} rc_RxPackage;

#define RC_SBUS_FRAME_SIZE 23
#define RC_SBUS_BUFFER_SIZE (RC_SBUS_FRAME_SIZE * 3)

typedef struct rc_Rc {
    volatile uint8_t rxDataBuffer[RC_SBUS_BUFFER_SIZE];
    volatile uint8_t rxLastValidFrameEnd;
    volatile uint8_t rxPreviousFrameEnd;
    UART_HandleTypeDef* huart;
} rc_Rc;

void rc_Init(rc_Rc* rc, UART_HandleTypeDef* huart);

bool rc_GetData(rc_Rc* rc, rc_RxPackage* data);

#endif // RC_H
