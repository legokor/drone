#include "tel/tel.h"
#include "err/err.h"
#include "log/log.h"

#include <stddef.h>
#include <string.h>

#define MAX_SOURCE_COUNT 20

// compacted list of sources
static tel_WriteFn _tel_sources[MAX_SOURCE_COUNT] = { 0 };

void tel_init(void) {
    log_debug("Telemetry initialized");
}

void tel_addSource(tel_WriteFn writeFn) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        if (_tel_sources[i] == NULL) {
            _tel_sources[i] = writeFn;
            return;
        }
    }

    err_fatal("Ran out of telemetry sources");
}

void tel_removeSource(tel_WriteFn writeFn) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        if (_tel_sources[i] == writeFn) {
            _tel_sources[i] = NULL;

            // compact
            for (size_t j = i; j < MAX_SOURCE_COUNT; j++) {
                _tel_sources[j - 1] = _tel_sources[j];

                if (_tel_sources[j] != NULL)
                    break;
            }

            return;
        }
    }

    err_ignorable("Ran out of telemetry sources");
}

static void _tel_writeMessage(tel_Topic topic, const void* data, size_t len, tel_DataType type) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        tel_WriteFn fn = _tel_sources[i];

        if (fn == NULL)
            break;

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
