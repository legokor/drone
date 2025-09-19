#include "uart/uart.h"

#include "irq/irq.h"
#include "log/log.h"

#include <stdlib.h>
#include <string.h>

void _uart_handleTransmitCplt(void* context) {
    uart_Uart* uart = (uart_Uart*) context;
    if (uart->txStartOfData == -1) {
        uart->txInProgress = false;
        return;
    }

    int charCount;
    if (uart->txStartOfData <= uart->txEndOfData) {
        charCount = uart->txEndOfData - uart->txStartOfData + 1;
        HAL_UART_Transmit_DMA(uart->huart, (uint8_t*) uart->txCircularBuffer + uart->txStartOfData, charCount);
        uart->txStartOfData = -1;
    } else {
        charCount = uart->txBufferLength - uart->txStartOfData;
        HAL_UART_Transmit_DMA(uart->huart, (uint8_t*) uart->txCircularBuffer + uart->txStartOfData, charCount);
        uart->txStartOfData = 0;
    }
}

bool uart_init(uart_Uart* uart, uart_UartInitParams uartInitParams) {
    log_debug("Initializing uart...");

    uart->huart = uartInitParams.huart;
    uart->txBufferLength = uartInitParams.txBufferLength;
    uart->rxBufferLength = uartInitParams.rxBufferLength;
    uart->uartIrq = uartInitParams.uartIrq;

    uart->txStartOfData = -1;
    uart->txEndOfData = uartInitParams.txBufferLength - 1;
    uart->txInProgress = false;
    uart->ignorableChars = uartInitParams.ignorableChars;
    uart->endOfMsgChar = uartInitParams.endOfMsgChar;
    uart->rxStartOfData = 0;

    uart->txCircularBuffer = (char*) malloc(uartInitParams.txBufferLength + 1);
    if (uart->txCircularBuffer == NULL) {
        log_error("Failed to allocate memory for txCircularBuffer");
        return false;
    }

    uart->rxCircularBuffer = (char*) malloc(uartInitParams.rxBufferLength);
    if (uart->rxCircularBuffer == NULL) {
        log_error("Failed to allocate memory for rxCircularBuffer");
        return false;
    }

    if (HAL_UART_Receive_DMA(uart->huart, (uint8_t*) uart->rxCircularBuffer, uartInitParams.rxBufferLength) != HAL_OK) {
        log_error("Failed to start UART receive DMA");
        return false;
    }

    irq_subscribeToIrq(irq_UART_TX_CPLT, _uart_handleTransmitCplt, uart, uart->huart);

    log_debug("UART initialized successfully");

    return true;
}

bool uart_transmit(uart_Uart* uart, const char* data, const uint32_t size) {
    if (size > uart->txBufferLength || size == 0)
        return false;

    bool ok = true;

    uint32_t spaceTillBufferEnd = uart->txBufferLength - uart->txEndOfData - 1;

    if (spaceTillBufferEnd >= size) {
        memcpy((void*) (uart->txCircularBuffer + uart->txEndOfData + 1), (const void*) data, size);
        HAL_NVIC_DisableIRQ(uart->uartIrq);

        if (uart->txStartOfData == -1) {
            if (uart->txInProgress) {
                uart->txStartOfData = uart->txEndOfData + 1;
            } else {
                const uint8_t* p = (const uint8_t*) uart->txCircularBuffer + uart->txEndOfData + 1;
                ok = HAL_UART_Transmit_DMA(uart->huart, p, size) == HAL_OK;

                uart->txInProgress = true;
            }
        }

        uart->txEndOfData = uart->txEndOfData + size;
        HAL_NVIC_EnableIRQ(uart->uartIrq);
    } else {
        if (spaceTillBufferEnd > 0) {
            memcpy((void*) (uart->txCircularBuffer + uart->txEndOfData + 1), (const void*) data, spaceTillBufferEnd);
        }

        memcpy((void*) uart->txCircularBuffer, (const void*) (data + spaceTillBufferEnd), size - spaceTillBufferEnd);
        HAL_NVIC_DisableIRQ(uart->uartIrq);

        if (uart->txStartOfData == -1) {
            if (spaceTillBufferEnd == 0) {
                if (uart->txInProgress) {
                    uart->txStartOfData = 0;
                } else {
                    uart->txInProgress = true;
                    ok = HAL_UART_Transmit_DMA(uart->huart, (uint8_t*) uart->txCircularBuffer, size) == HAL_OK;
                }

                uart->txEndOfData = size - 1;
            } else {
                if (uart->txInProgress) {
                    uart->txStartOfData = uart->txEndOfData + 1;
                } else {
                    uart->txInProgress = true;
                    uart->txStartOfData = 0;

                    const uint8_t* p = (const uint8_t*) uart->txCircularBuffer + uart->txEndOfData + 1;
                    ok = HAL_UART_Transmit_DMA(uart->huart, p, spaceTillBufferEnd) == HAL_OK;
                }

                uart->txEndOfData = size - spaceTillBufferEnd - 1;
            }

        } else {
            uart->txEndOfData = size - spaceTillBufferEnd - 1;
        }

        HAL_NVIC_EnableIRQ(uart->uartIrq);
    }

    return ok;
}

uart_ReceiveStatus uart_receive(uart_Uart* uart, char* data, uint32_t maxSize) {
    uint32_t dmaPtr = uart->rxBufferLength - uart->huart->hdmarx->Instance->NDTR;

    uart_ReceiveStatus status = { .size = 0, .eomReached = false };
    while (dmaPtr != uart->rxStartOfData) {
        char c = uart->rxCircularBuffer[uart->rxStartOfData];

        if (c == uart->endOfMsgChar)
            status.eomReached = true;

        if (status.size < maxSize || status.eomReached) {
            uart->rxStartOfData++;

            if (uart->rxStartOfData == uart->rxBufferLength)
                uart->rxStartOfData = 0;
        }

        if (status.eomReached || status.size >= maxSize)
            break;

        if (uart->ignorableChars != NULL) {
            bool ignore = false;

            for (uint32_t j = 0; uart->ignorableChars[j] != '\0'; j++) {
                if (c == uart->ignorableChars[j]) {
                    ignore = true;
                    break;
                }
            }

            if (ignore)
                continue;
        }

        data[status.size++] = c;
    }

    return status;
}
