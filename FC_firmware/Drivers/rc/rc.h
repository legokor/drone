#ifndef RC_H
#define RC_H

#include <stdbool.h>
#include "stm32f4xx_hal.h"

typedef struct rc_RxPackage {
    uint16_t channels[18];
    bool frameLost;
    bool failsafeActive;
} rc_RxPackage;

#define RC_SBUS_FRAME_SIZE 25

typedef enum { RC_STATE_WAIT_FOR_START, RC_STATE_RECEIVING } rc_SbusState;

typedef struct rc_Rc {
    volatile uint8_t rxDataBuffer[RC_SBUS_FRAME_SIZE];
    volatile bool frameValid;
    uint32_t lastFrameTime;
    rc_SbusState state;
    UART_HandleTypeDef* huart;
} rc_Rc;

void rc_Init(rc_Rc* rc, UART_HandleTypeDef* huart);

bool rc_GetData(rc_Rc* rc, rc_RxPackage* data);

#endif // RC_H
