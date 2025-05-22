#ifndef INT_INT_H_
#define INT_INT_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum int_IntEventType {
    INT_UART_TX_CPLT = 0,
    INT_UART_RX_CPLT,
    INT_UART_RX_EVENT,
    INT_UART_ERROR,

    INT_SPI_TX_CPLT,
    INT_SPI_RX_CPLT,

    INT_TIM_PERIOD_ELAPSED,

    INT_EVENT_TYPE_COUNT
} int_IntEventType;

typedef void (*int_CallbackFn)(void*);

bool int_subscribeToInt(int_IntEventType eventType, int_CallbackFn cbFnHandle, void* context, void* halHandle);

#endif /* INT_INT_H_ */
