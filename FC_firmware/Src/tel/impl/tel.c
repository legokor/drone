#include "tel/tel.h"
#include <string.h>
#include "log/log.h"
#include "sys/sys.h"
#include "utils/utils.h"

#define MAX_SOURCE_COUNT 20

static tel_WriteFn _tel_sources[MAX_SOURCE_COUNT] = { 0 };

static void _tel_writeUart(uint32_t t, tel_Topic topic, const void* data, size_t len, tel_DataType type) {
    // TODO: move + packetize
    uart_transmit(&sys_uartInstance, data, len);
}

void tel_init(void) {
    tel_addSource(_tel_writeUart);

    log_debug("Telemetry initialized");
}

void tel_addSource(tel_WriteFn writeFn) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        if (_tel_sources[i] == NULL) {
            _tel_sources[i] = writeFn;
            return;
        }
    }

    // TODO: error
}

void tel_removeSource(tel_WriteFn writeFn) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        if (_tel_sources[i] == writeFn) {
            _tel_sources[i] = NULL;
            return;
        }
    }

    // TODO: error
}

static void _tel_writeMessage(tel_Topic topic, const void* data, size_t len, tel_DataType type) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        tel_WriteFn fn = _tel_sources[i];

        if (fn == NULL)
            continue;

        fn(HAL_GetTick(), topic, data, len, type);
    }
}

void tel_writePing(tel_Topic topic) {
    _tel_writeMessage(topic, NULL, 0, tel_TYPE_PING);
}

void tel_writeInt(tel_Topic topic, uint32_t i) {
    _tel_writeMessage(topic, &i, sizeof(i), tel_TYPE_INTEGER);
}

void tel_writeFloat(tel_Topic topic, double d) {
    _tel_writeMessage(topic, &d, sizeof(d), tel_TYPE_FLOAT);
}

void tel_writeChar(tel_Topic topic, char c) {
    _tel_writeMessage(topic, &c, 1, tel_TYPE_CHAR);
}

void tel_writeBool(tel_Topic topic, bool b) {
    _tel_writeMessage(topic, &b, sizeof(b), tel_TYPE_BOOLEAN);
}

static uint8_t _tel_getDataSize(tel_DataType type) {
    switch (type) {
        case tel_TYPE_PING: return 0;
        case tel_TYPE_INTEGER: return 4;
        case tel_TYPE_FLOAT: return 8;
        case tel_TYPE_CHAR: return 1;
        case tel_TYPE_BOOLEAN: return 1;
        default: return -1;
    }
}

void tel_writeArray(tel_Topic topic, const void* arr, uint8_t len, tel_DataType type) {
    _tel_writeMessage(topic, arr, len * _tel_getDataSize(type), tel_TYPE_ARRAY);
}

void tel_writeString(tel_Topic topic, const char* str) {
    tel_writeArray(topic, str, strlen(str), tel_TYPE_CHAR);
}
