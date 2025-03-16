#include "uart/uart.h"
#include <string.h>
#include "int/int.h"
#include "log/log.h"

void _uart_HandleTransmitCplt(void* context) {
    uart_Uart* uart = (uart_Uart*) context;
    if (uart->txStartOfData == -1) {
        uart->txInProgress = 0;
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

uint8_t uart_Init(uart_Uart* uart, uart_UartInitParams uartInitParams) {
    log_Debug("Initializing uart...");

    uart->huart = uartInitParams.huart;
    uart->txBufferLength = uartInitParams.txBufferLength;
    uart->rxBufferLength = uartInitParams.rxBufferLength;
    uart->uartIr = uartInitParams.uartIr;

    uart->txStartOfData = -1;
    uart->txEndOfData = uartInitParams.txBufferLength - 1;
    uart->txInProgress = 0;
    uart->ignorableChars = uartInitParams.ignorableChars;
    uart->endOfMsgChar = uartInitParams.endOfMsgChar;
    uart->rxStartOfData = 0;

    uart->txCircularBuffer = (char*) malloc(uartInitParams.txBufferLength + 1);
    if (uart->txCircularBuffer == NULL) {
        log_Error("Failed to allocate memory for txCircularBuffer");
        return 0;
    }
    uart->rxCircularBuffer = (char*) malloc(uartInitParams.rxBufferLength);
    if (uart->rxCircularBuffer == NULL) {
        log_Error("Failed to allocate memory for rxCircularBuffer");
        return 0;
    }

    if (HAL_UART_Receive_DMA(uart->huart, (uint8_t*) uart->rxCircularBuffer, uartInitParams.rxBufferLength) != HAL_OK) {
        log_Error("Failed to start UART receive DMA");
        return 0;
    }

    if (!int_SubscribeToInt(INT_UART_TX_CPLT, _uart_HandleTransmitCplt, uart, uart->huart)) {
        log_Error("Failed to subscribe to INT_UART_TX_CPLT");
        return 0;
    }

    log_Debug("UART initialized successfully");
    return 1;
}

uint8_t uart_Transmit(uart_Uart* uart, const char* data, const uint32_t size) {
    if (size > uart->txBufferLength || size == 0)
        return 0;

    uint8_t ok = 1;

    int spaceTillBufferEnd = uart->txBufferLength - uart->txEndOfData - 1;

    if (spaceTillBufferEnd >= size) {
        memcpy((void*) uart->txCircularBuffer + uart->txEndOfData + 1, (const void*) data, size);
        HAL_NVIC_DisableIRQ(uart->uartIr);
        if (uart->txStartOfData == -1) {
            if (uart->txInProgress) {
                uart->txStartOfData = uart->txEndOfData + 1;
            } else {
                if (HAL_UART_Transmit_DMA(uart->huart, (uint8_t*) uart->txCircularBuffer + uart->txEndOfData + 1,
                                          size) != HAL_OK)
                    ok = 0;
                uart->txInProgress = 1;
            }
        }
        uart->txEndOfData = uart->txEndOfData + size;
        HAL_NVIC_EnableIRQ(uart->uartIr);
    } else {
        if (spaceTillBufferEnd > 0)
            memcpy((void*) uart->txCircularBuffer + uart->txEndOfData + 1, (const void*) data, spaceTillBufferEnd);
        memcpy((void*) uart->txCircularBuffer, (const void*) data + spaceTillBufferEnd, size - spaceTillBufferEnd);
        HAL_NVIC_DisableIRQ(uart->uartIr);
        if (uart->txStartOfData == -1) {
            if (spaceTillBufferEnd == 0) {
                if (uart->txInProgress) {
                    uart->txStartOfData = 0;
                } else {
                    uart->txInProgress = 1;
                    if (HAL_UART_Transmit_DMA(uart->huart, (uint8_t*) uart->txCircularBuffer, size) != HAL_OK)
                        ok = 0;
                }
                uart->txEndOfData = size - 1;
            } else {
                if (uart->txInProgress) {
                    uart->txStartOfData = uart->txEndOfData + 1;
                } else {
                    uart->txInProgress = 1;
                    uart->txStartOfData = 0;
                    if (HAL_UART_Transmit_DMA(uart->huart, (uint8_t*) uart->txCircularBuffer + uart->txEndOfData + 1,
                                              spaceTillBufferEnd) != HAL_OK)
                        ok = 0;
                }
                uart->txEndOfData = size - spaceTillBufferEnd - 1;
            }
        } else {
            uart->txEndOfData = size - spaceTillBufferEnd - 1;
        }
        HAL_NVIC_EnableIRQ(uart->uartIr);
    }

    return ok;
}

uart_ReceiveStatus uart_Receive(uart_Uart* uart, char* data, uint32_t maxSize) {
    uint32_t dmaPtr = uart->rxBufferLength - uart->huart->hdmarx->Instance->NDTR;

    uart_ReceiveStatus status = { 0, 0 };
    uint32_t i = 0;
    while (dmaPtr != uart->rxStartOfData) {
        char c = uart->rxCircularBuffer[uart->rxStartOfData];
        if (c == uart->endOfMsgChar) {
            status.eomReached = 1;
        }

        if (i < maxSize || status.eomReached == 1) {
            uart->rxStartOfData++;
            if (uart->rxStartOfData == uart->rxBufferLength)
                uart->rxStartOfData = 0;
        }
        if (status.eomReached == 1) {
            break;
        }

        if (i >= maxSize) {
            break;
        }

        if (uart->ignorableChars != NULL) {
            uint8_t ignore = 0;
            for (uint32_t j = 0; uart->ignorableChars[j] != '\0'; j++) {
                if (c == uart->ignorableChars[j]) {
                    ignore = 1;
                    break;
                }
            }
            if (ignore)
                continue;
        }
        data[i++] = c;
    }

    status.size = i;
    return status;
}
