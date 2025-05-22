#include "tel/tel.h"
#include <string.h>
#include "log/log.h"
#include "sys/sys.h"
#include "utils/utils.h"

#define MAX_SOURCE_COUNT 20

static tel_WriteFn _tel_sources[MAX_SOURCE_COUNT] = { 0 };

static void _tel_writeUart(const void* data, size_t len) {
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
}

void tel_removeSource(tel_WriteFn writeFn) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        if (_tel_sources[i] == writeFn) {
            _tel_sources[i] = NULL;
            return;
        }
    }
}

static void _tel_writeBytes(const void* data, size_t len) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        tel_WriteFn fn = _tel_sources[i];

        if (fn == NULL)
            continue;

        fn(data, len);
    }
}

static void _tel_writeMessage(tel_Topic topic, const void* data, size_t len, tel_DataType type) {
    // uint32_t t = utils_getMsSinceStartup(); // TODO: uncomment this when the telemetry software is ready
    // _tel_writeBytes(&t, 4);
    // _tel_writeBytes(&type, 1);
    // _tel_writeBytes(&topic, 1);
    _tel_writeBytes(data, len);
}

void tel_writePing(tel_Topic topic) {
    _tel_writeMessage(topic, NULL, 0, tel_TYPE_PING);
}

void tel_writeInteger(tel_Topic topic, uint32_t i) {
    _tel_writeMessage(topic, &i, 4, tel_TYPE_INTEGER);
}

void tel_writeFloat(tel_Topic topic, double d) {
    _tel_writeMessage(topic, &d, 8, tel_TYPE_FLOAT);
}

void tel_writeChar(tel_Topic topic, char c) {
    _tel_writeMessage(topic, &c, 1, tel_TYPE_CHAR);
}

void tel_writeBoolean(tel_Topic topic, bool b) {
    _tel_writeMessage(topic, &b, 1, tel_TYPE_BOOLEAN);
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
