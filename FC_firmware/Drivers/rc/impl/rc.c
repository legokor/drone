#include "rc/rc.h"
#include <string.h>
#include "int/int.h"
#include "log/log.h"
#include "stm32f4xx_hal_dma.h"

#define RC_SBUS_CH17_MASK 0x01
#define RC_SBUS_CH18_MASK 0x02
#define RC_SBUS_LOST_FRAME_MASK 0x10
#define RC_SBUS_FAILSAFE_MASK 0x20

static void rc_HandleRxIdle(void* context) {
    rc_Rc* rc = (rc_Rc*) context;
    uint8_t end = rc->huart->RxXferSize - rc->huart->RxXferCount;
    uint8_t diff = end > rc->rxPreviousFrameEnd ? end - rc->rxPreviousFrameEnd
                                                : RC_SBUS_BUFFER_SIZE - rc->rxPreviousFrameEnd + end;
    rc->rxPreviousFrameEnd = end;
    rc->rxLastValidFrameEnd = diff == RC_SBUS_FRAME_SIZE ? end : 0xff;
}

void rc_Init(rc_Rc* rc, UART_HandleTypeDef* huart) {
    log_Debug("Initializing rc...");

    rc->huart = huart;
    rc->rxLastValidFrameEnd = 0xff;
    rc->rxPreviousFrameEnd = 0;
    for (uint32_t i = 0; i < RC_SBUS_BUFFER_SIZE; i++) {
        rc->rxDataBuffer[i] = 0;
    }

    int_SubscribeToInt(INT_UART_RX_EVENT, rc_HandleRxIdle, rc, huart);

    HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t*) rc->rxDataBuffer, RC_SBUS_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT); // Disable half transfer interrupt
    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_TC); // Disable transfer complete interrupt
}

bool rc_GetData(rc_Rc* rc, rc_RxPackage* data) {
    uint8_t end = rc->rxLastValidFrameEnd;
    if (end == 0xff) {
        return false;
    }

    uint8_t buffer[RC_SBUS_FRAME_SIZE + 1];
    if (end >= RC_SBUS_FRAME_SIZE) {
        memcpy(&buffer[1], (uint8_t*) rc->rxDataBuffer + end - RC_SBUS_FRAME_SIZE, RC_SBUS_FRAME_SIZE);
    } else {
        memcpy(&buffer[1], (uint8_t*) rc->rxDataBuffer + RC_SBUS_BUFFER_SIZE - RC_SBUS_FRAME_SIZE + end,
               RC_SBUS_FRAME_SIZE - end);
        memcpy(&buffer[1] + RC_SBUS_FRAME_SIZE - end, (uint8_t*) rc->rxDataBuffer, end);
    }

    data->channels[0] = (buffer[1] | ((buffer[2] << 8) & 0x07FF));
    data->channels[1] = ((buffer[2] >> 3) | ((buffer[3] << 5) & 0x07FF));
    data->channels[2] = ((buffer[3] >> 6) | (buffer[4] << 2) | ((buffer[5] << 10) & 0x07FF));
    data->channels[3] = ((buffer[5] >> 1) | ((buffer[6] << 7) & 0x07FF));
    data->channels[4] = ((buffer[6] >> 4) | ((buffer[7] << 4) & 0x07FF));
    data->channels[5] = ((buffer[7] >> 7) | (buffer[8] << 1) | ((buffer[9] << 9) & 0x07FF));
    data->channels[6] = ((buffer[9] >> 2) | ((buffer[10] << 6) & 0x07FF));
    data->channels[7] = ((buffer[10] >> 5) | ((buffer[11] << 3) & 0x07FF));
    data->channels[8] = (buffer[12] | ((buffer[13] << 8) & 0x07FF));
    data->channels[9] = ((buffer[13] >> 3) | ((buffer[14] << 5) & 0x07FF));
    data->channels[10] = ((buffer[14] >> 6) | (buffer[15] << 2) | ((buffer[16] << 10) & 0x07FF));
    data->channels[11] = ((buffer[16] >> 1) | ((buffer[17] << 7) & 0x07FF));
    data->channels[12] = ((buffer[17] >> 4) | ((buffer[18] << 4) & 0x07FF));
    data->channels[13] = ((buffer[18] >> 7) | (buffer[19] << 1) | ((buffer[20] << 9) & 0x07FF));
    data->channels[14] = ((buffer[20] >> 2) | ((buffer[21] << 6) & 0x07FF));
    data->channels[15] = ((buffer[21] >> 5) | ((buffer[22] << 3) & 0x07FF));

    data->channels[16] = (buffer[23] & RC_SBUS_CH17_MASK) > 0;
    data->channels[17] = (buffer[23] & RC_SBUS_CH18_MASK) > 0;
    data->frameLost = (buffer[23] & RC_SBUS_LOST_FRAME_MASK) > 0;
    data->failsafeActive = (buffer[23] & RC_SBUS_FAILSAFE_MASK) > 0;

    return true;
}
