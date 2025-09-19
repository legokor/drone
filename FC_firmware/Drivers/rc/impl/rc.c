#include "rc/rc.h"
#include "irq/irq.h"
#include "log/log.h"
#include "utils/utils.h"

#include "stm32f4xx_hal_dma.h"

#include <string.h>

#define _rc_SBUS_CH17_MASK 0x01
#define _rc_SBUS_CH18_MASK 0x02
#define _rc_SBUS_LOST_FRAME_MASK 0x10
#define _rc_SBUS_FAILSAFE_MASK 0x20
#define _rc_SBUS_FRAME_START 0x0F
#define _rc_SBUS_FRAME_END 0x00
#define _rc_SBUS_MIN_TIME_BETWEEN_FRAMES 6

#define _rc_SBUS_CHAN_BITS 11
#define _rc_SBUS_CHAN_BYTES (_rc_SBUS_CHAN_BITS / 8 + utils_MIN(_rc_SBUS_CHAN_BITS % 8, 2))
#define _rc_SBUS_CHANNELS 16
#define _rc_SBUS_MASK ((1 << _rc_SBUS_CHAN_BITS) - 1)

static void _rc_handleRxCplt(void* context) {
    rc_Rc* rc = (rc_Rc*) context;
    uint32_t currentTime = HAL_GetTick();

    if (rc->state == rc_STATE_WAIT_FOR_START) {
        if (currentTime - rc->lastFrameTime >= _rc_SBUS_MIN_TIME_BETWEEN_FRAMES &&
            rc->rxDMABuffer[0] == _rc_SBUS_FRAME_START) {
            rc->state = rc_STATE_RECEIVING;

            HAL_UART_Receive_DMA(rc->huart, (uint8_t*) rc->rxDMABuffer + 1, rc_SBUS_FRAME_SIZE - 1);

            // HAL enables it every time...
            __HAL_DMA_DISABLE_IT(rc->huart->hdmarx, DMA_IT_HT);
        } else {
            HAL_UART_Receive_IT(rc->huart, (uint8_t*) rc->rxDMABuffer, 1);
            rc->frameValid = false;
        }
    } else if (rc->state == rc_STATE_RECEIVING) {
        rc->frameValid = rc->rxDMABuffer[rc_SBUS_FRAME_SIZE - 1] == _rc_SBUS_FRAME_END;
        rc->state = rc_STATE_WAIT_FOR_START;

        memcpy((uint8_t*) rc->rxDataBuffer, (uint8_t*) rc->rxDMABuffer, rc_SBUS_FRAME_SIZE);

        HAL_UART_Receive_IT(rc->huart, (uint8_t*) rc->rxDMABuffer, 1);
    }

    rc->lastFrameTime = currentTime;
}

void rc_init(rc_Rc* rc, UART_HandleTypeDef* huart) {
    log_debug("Initializing rc...");

    rc->huart = huart;
    rc->frameValid = false;
    rc->state = rc_STATE_WAIT_FOR_START;
    rc->lastFrameTime = HAL_GetTick();

    irq_subscribeToIrq(irq_UART_RX_CPLT, _rc_handleRxCplt, rc, huart);

    HAL_UART_Receive_IT(huart, (uint8_t*) rc->rxDataBuffer, 1);
}

// aaaaaaaa
// aaabbbbb
// bbbbbbcc
// cccccccc
// cddddddd
// dddd

// aaaaaaaaaaa
// bbbbbbbbbbb
// ccccccccccc
// ddddddddddd
static void _rc_parseData(const uint8_t* restrict buff, uint16_t* channels) {
    // same as the hard-coded version on O3

    for (size_t c = 0; c < _rc_SBUS_CHANNELS; c++) {
        size_t bit_pos = c * _rc_SBUS_CHAN_BITS;
        size_t bit_offset = bit_pos % 8;
        size_t byte_pos = bit_pos / 8;

        uint32_t d = 0;
        for (int i = 0; i < _rc_SBUS_CHAN_BYTES; i++)
            d |= buff[byte_pos + i] << (i * 8);

        channels[c] = (d >> bit_offset) & _rc_SBUS_MASK;
    }
}

bool rc_getData(rc_Rc* rc, rc_RxPackage* data) {
    if (!rc->frameValid) {
        return false;
    }

    HAL_NVIC_DisableIRQ(rc->writeIrq);

    uint8_t buff[rc_SBUS_FRAME_SIZE];
    memcpy(buff, (uint8_t*) rc->rxDataBuffer, rc_SBUS_FRAME_SIZE);

    HAL_NVIC_EnableIRQ(rc->writeIrq);

    _rc_parseData(buff, data->channels);

    // data->channels[0] = (buff[1] | ((buff[2] << 8) & 0x07FF));
    // data->channels[1] = ((buff[2] >> 3) | ((buff[3] << 5) & 0x07FF));
    // data->channels[2] = ((buff[3] >> 6) | (buff[4] << 2) | ((buff[5] << 10) & 0x07FF));
    // data->channels[3] = ((buff[5] >> 1) | ((buff[6] << 7) & 0x07FF));
    // data->channels[4] = ((buff[6] >> 4) | ((buff[7] << 4) & 0x07FF));
    // data->channels[5] = ((buff[7] >> 7) | (buff[8] << 1) | ((buff[9] << 9) & 0x07FF));
    // data->channels[6] = ((buff[9] >> 2) | ((buff[10] << 6) & 0x07FF));
    // data->channels[7] = ((buff[10] >> 5) | ((buff[11] << 3) & 0x07FF));
    // data->channels[8] = (buff[12] | ((buff[13] << 8) & 0x07FF));
    // data->channels[9] = ((buff[13] >> 3) | ((buff[14] << 5) & 0x07FF));
    // data->channels[10] = ((buff[14] >> 6) | (buff[15] << 2) | ((buff[16] << 10) & 0x07FF));
    // data->channels[11] = ((buff[16] >> 1) | ((buff[17] << 7) & 0x07FF));
    // data->channels[12] = ((buff[17] >> 4) | ((buff[18] << 4) & 0x07FF));
    // data->channels[13] = ((buff[18] >> 7) | (buff[19] << 1) | ((buff[20] << 9) & 0x07FF));
    // data->channels[14] = ((buff[20] >> 2) | ((buff[21] << 6) & 0x07FF));
    // data->channels[15] = ((buff[21] >> 5) | ((buff[22] << 3) & 0x07FF));

    data->channels[16] = (buff[23] & _rc_SBUS_CH17_MASK) > 0;
    data->channels[17] = (buff[23] & _rc_SBUS_CH18_MASK) > 0;
    data->frameLost = (buff[23] & _rc_SBUS_LOST_FRAME_MASK) > 0;
    data->failsafeActive = (buff[23] & _rc_SBUS_FAILSAFE_MASK) > 0;

    return true;
}
