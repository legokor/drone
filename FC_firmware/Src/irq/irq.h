#ifndef INT_INT_H_
#define INT_INT_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum [[nodiscard]] irq_IntEventType {
    irq_UART_TX_CPLT = 0,
    irq_UART_RX_CPLT,
    irq_UART_RX_EVENT,
    irq_UART_ERROR,

    irq_SPI_TX_CPLT,
    irq_SPI_RX_CPLT,

    irq_TIM_PERIOD_ELAPSED,

    _irq_EVENT_TYPE_COUNT
} irq_IntEventType;

typedef void (*irq_CallbackFn)(void*);

void irq_subscribeToIrq(irq_IntEventType eventType, irq_CallbackFn cbFnHandle, void* context, void* halHandle);

#endif /* INT_INT_H_ */
