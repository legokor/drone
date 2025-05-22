#ifndef RC_H
#define RC_H

#include <stdbool.h>
#include "stm32f4xx_hal.h"

typedef struct rc_RxPackage {
    uint16_t channels[18];
    bool frameLost;
    bool failsafeActive;
} rc_RxPackage;

#define rc_SBUS_FRAME_SIZE 25

typedef enum { rc_STATE_WAIT_FOR_START, rc_STATE_RECEIVING } rc_SbusState;

typedef struct rc_Rc {
    UART_HandleTypeDef* huart;
    IRQn_Type writeIrq;

    // not volatile, becase we only access it from the interrupt
    uint8_t rxDMABuffer[rc_SBUS_FRAME_SIZE];

    // but we access this from both interrupts and normal code
    volatile uint8_t rxDataBuffer[rc_SBUS_FRAME_SIZE];
    volatile bool frameValid;

    uint32_t lastFrameTime;
    rc_SbusState state;
} rc_Rc;

void rc_init(rc_Rc* rc, UART_HandleTypeDef* huart);

bool rc_getData(rc_Rc* rc, rc_RxPackage* data);

#endif // RC_H
