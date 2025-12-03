#ifndef RC_H
#define RC_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

#define rc_CHANNEL_COUNT 18

typedef struct [[nodiscard]] rc_RxPackage {
    uint16_t channels[rc_CHANNEL_COUNT];
    bool frameLost, failsafeActive;
} rc_RxPackage;

#define _rc_SBUS_FRAME_SIZE 25

typedef enum [[nodiscard]] rc_SbusState {
    rc_STATE_WAIT_FOR_START,
    rc_STATE_RECEIVING,
} rc_SbusState;

typedef struct [[nodiscard]] rc_Rc {
    UART_HandleTypeDef* huart;
    IRQn_Type writeIrq;

    // not volatile, becase we only access it from the interrupt
    uint8_t rxDMABuffer[_rc_SBUS_FRAME_SIZE];

    // but we access this from both interrupts and normal code
    volatile uint8_t rxDataBuffer[_rc_SBUS_FRAME_SIZE];
    volatile bool frameValid;

    uint32_t lastFrameTime;
    rc_SbusState state;
} rc_Rc;

[[nodiscard]] bool rc_init(rc_Rc* rc, UART_HandleTypeDef* huart);

[[nodiscard]] bool rc_getData(const rc_Rc* rc, rc_RxPackage* data);

#endif // RC_H
