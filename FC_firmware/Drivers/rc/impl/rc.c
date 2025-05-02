#include "rc/rc.h"
#include <string.h>
#include "int/int.h"
#include "log/log.h"
#include "stm32f4xx_hal_dma.h"

#define RC_SBUS_CH17_MASK 0x01
#define RC_SBUS_CH18_MASK 0x02
#define RC_SBUS_LOST_FRAME_MASK 0x10
#define RC_SBUS_FAILSAFE_MASK 0x20
#define RC_SBUS_FRAME_START 0x0F
#define RC_SBUS_FRAME_END 0x00
#define RC_SBUS_MIN_TIME_BETWEEN_FRAMES 6

static void _rc_HandleRxCplt(void* context) {
    rc_Rc* rc = (rc_Rc*) context;
    uint32_t currentTime = HAL_GetTick();
    if (rc->state == RC_STATE_WAIT_FOR_START) {
        if (currentTime - rc->lastFrameTime >= RC_SBUS_MIN_TIME_BETWEEN_FRAMES &&
            rc->rxDataBuffer[0] == RC_SBUS_FRAME_START) {
            rc->state = RC_STATE_RECEIVING;
            HAL_UART_Receive_DMA(rc->huart, (uint8_t*) rc->rxDataBuffer + 1, RC_SBUS_FRAME_SIZE - 1);
            __HAL_DMA_DISABLE_IT(rc->huart->hdmarx, DMA_IT_HT);
        } else {
            HAL_UART_Receive_IT(rc->huart, (uint8_t*) rc->rxDataBuffer, 1);
            rc->frameValid = false;
        }
    } else if (rc->state == RC_STATE_RECEIVING) {
        rc->frameValid = rc->rxDataBuffer[RC_SBUS_FRAME_SIZE - 1] == RC_SBUS_FRAME_END;
        rc->state = RC_STATE_WAIT_FOR_START;
        HAL_UART_Receive_IT(rc->huart, (uint8_t*) rc->rxDataBuffer, 1);
    }
    rc->lastFrameTime = currentTime;
}

void rc_Init(rc_Rc* rc, UART_HandleTypeDef* huart) {
    log_Debug("Initializing rc...");

    rc->huart = huart;
    rc->frameValid = false;
    rc->lastFrameTime = HAL_GetTick();
    rc->state = RC_STATE_WAIT_FOR_START;

    int_SubscribeToInt(INT_UART_RX_CPLT, _rc_HandleRxCplt, rc, huart);

    HAL_UART_Receive_IT(huart, (uint8_t*) rc->rxDataBuffer, 1);
}

bool rc_GetData(rc_Rc* rc, rc_RxPackage* data) {
    if (!rc->frameValid) {
        return false;
    }

    uint8_t buffer[RC_SBUS_FRAME_SIZE];
    memcpy(buffer, (uint8_t*) rc->rxDataBuffer, RC_SBUS_FRAME_SIZE);

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
