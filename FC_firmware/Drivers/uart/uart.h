#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

typedef struct [[nodiscard]] uart_UartInitParams {
    UART_HandleTypeDef* huart;
    IRQn_Type uartIrq;

    uint16_t rxBufferLength, txBufferLength;

    // ignore these characters when receiving data
    const char* ignorableChars;

    // end of message character, it is not included in the received data
    char endOfMsgChar;
} uart_UartInitParams;

typedef struct [[nodiscard]] uart_Uart {
    UART_HandleTypeDef* huart;
    IRQn_Type uartIrq, txDmaIrq;

    uint16_t rxBufferLength, txBufferLength;

    volatile char* txCircularBuffer;
    volatile int32_t txStartOfData;
    volatile int32_t txEndOfData;
    volatile bool txInProgress;

    volatile char* rxCircularBuffer;
    uint32_t rxStartOfData;

    const char* ignorableChars;
    char endOfMsgChar;
} uart_Uart;

typedef struct [[nodiscard]] uart_ReceiveStatus {
    uint32_t size;
    bool eomReached;
} uart_ReceiveStatus;

/**
 * @brief Initializes the UART instance using the parameters provided in the init struct
 * @param uart - a pointer to the UART instance struct
 * @param uartInitParams - the parameters that are used for initialization
 *
 * @return true on success, else false
 */
[[nodiscard]] bool uart_init(uart_Uart* uart, uart_UartInitParams uartInitParams);

/**
 * @brief Transmits the data using the UART instance, the size of the data must not exceed the write buffer length
 * @param uart - a pointer to the UART instance struct
 * @param data - the data to be transmitted
 * @param size - the size of the data to be transmitted
 *
 * @return true on success, else false
 */
[[nodiscard]] bool uart_transmit(uart_Uart* uart, const char* data, uint32_t size);

/**
 * @brief Writes the received data to the data buffer
 * @param uart - a pointer to the UART instance struct
 * @param data - the data to be received (the end of message character is not included)
 * @param maxSize - the maximum size of the data to be received
 *
 * @return the size of the data received and a flag indicating if the end of message character was reached
 */
[[nodiscard]] uart_ReceiveStatus uart_receive(uart_Uart* uart, char* data, uint32_t maxSize);

#endif // UART_H
