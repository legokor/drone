#include "irq/impl/irq_list.h"
#include "stm32f4xx_hal.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    _irq_triggerCbs(irq_UART_RX_CPLT, (void*) huart);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
    _irq_triggerCbs(irq_UART_TX_CPLT, (void*) huart);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t pos) {
    UNUSED(pos);
    _irq_triggerCbs(irq_UART_RX_EVENT, (void*) huart);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart) {
    _irq_triggerCbs(irq_UART_ERROR, (void*) huart);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
    _irq_triggerCbs(irq_SPI_TX_CPLT, (void*) hspi);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi) {
    _irq_triggerCbs(irq_SPI_RX_CPLT, (void*) hspi);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    _irq_triggerCbs(irq_TIM_PERIOD_ELAPSED, (void*) htim);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef* htim) {
    _irq_triggerCbs(irq_TIM_PWM_DMA_CPLT, (void*) htim);
}
