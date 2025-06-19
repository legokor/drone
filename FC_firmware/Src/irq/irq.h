#ifndef INT_INT_H_
#define INT_INT_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum [[nodiscard]] int_IntEventType {
    int_UART_TX_CPLT = 0,
    int_UART_RX_CPLT,
    int_UART_RX_EVENT,
    int_UART_ERROR,

    int_SPI_TX_CPLT,
    int_SPI_RX_CPLT,

    int_TIM_PERIOD_ELAPSED,

    _int_EVENT_TYPE_COUNT
} int_IntEventType;

typedef void (*int_CallbackFn)(void*);

void int_subscribeToInt(int_IntEventType eventType, int_CallbackFn cbFnHandle, void* context, void* halHandle);

#endif /* INT_INT_H_ */
