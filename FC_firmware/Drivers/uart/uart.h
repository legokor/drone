#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

typedef struct uart_UartInitParams {
    UART_HandleTypeDef* huart;
    IRQn_Type uartIr;
    uint16_t txBufferLength;
    uint16_t rxBufferLength;
    const char* ignorableChars; // ignore these characters when receiving data
    char endOfMsgChar;          // end of message character, it is not included in the received data
} uart_UartInitParams;

typedef struct uart_Uart {
    UART_HandleTypeDef* huart;
    IRQn_Type uartIr;
    IRQn_Type txDmaIr;

    uint16_t txBufferLength;
    uint16_t rxBufferLength;

    volatile char* txCircularBuffer;
    volatile int32_t txStartOfData;
    volatile int32_t txEndOfData;
    volatile uint8_t txInProgress;

    volatile char* rxCircularBuffer;
    uint32_t rxStartOfData;

    const char* ignorableChars;
    char endOfMsgChar;
} uart_Uart;

typedef struct uart_ReceiveStatus {
    uint32_t size;
    uint8_t eomReached;
} uart_ReceiveStatus;

/**
 * @brief Initializes the UART instance using the parameters provided in the init struct
 * @param uart - a pointer to the UART instance struct
 * @param uartInitParams - the parameters that are used for initialization
 *
 * @return 1 on success, else 0
 */
uint8_t uart_Init(uart_Uart* uart, uart_UartInitParams uartInitParams);

/**
 * @brief Transmits the data using the UART instance, the size of the data must not exceed the write buffer length
 * @param uart - a pointer to the UART instance struct
 * @param data - the data to be transmitted
 * @param size - the size of the data to be transmitted
 *
 * @return 1 on success, else 0
 */
uint8_t uart_Transmit(uart_Uart* uart, const char* data, const uint32_t size);

/**
 * @brief Writes the received data to the data buffer
 * @param uart - a pointer to the UART instance struct
 * @param data - the data to be received (the end of message character is not included)
 * @param maxSize - the maximum size of the data to be received
 *
 * @return the size of the data received and a flag indicating if the end of message character was reached
 */
uart_ReceiveStatus uart_Receive(uart_Uart* uart, char* data, uint32_t maxSize);

#endif // UART_H
