#include "tel/tel.h"
#include <string.h>
#include "log/log.h"
#include "utils/utils.h"

#define MAX_SOURCE_COUNT 256
static tel_WriteFn _tel_Sources[MAX_SOURCE_COUNT] = { 0 };

void tel_Init(void) {
    log_Debug("Initalizing tel...");
}

void tel_AddSource(tel_WriteFn writeFn) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        if (_tel_Sources[i] == NULL) {
            _tel_Sources[i] = writeFn;
            return;
        }
    }
}

void tel_RemoveSource(tel_WriteFn writeFn) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        if (_tel_Sources[i] == writeFn) {
            _tel_Sources[i] = NULL;
            return;
        }
    }
}

static void _tel_WriteBytes(const void* data, size_t len) {
    for (size_t i = 0; i < MAX_SOURCE_COUNT; i++) {
        tel_WriteFn fn = _tel_Sources[i];

        if (fn == NULL)
            continue;

        fn(data, len);
    }
}
static void _tel_WriteMessage(tel_Topic topic, const void* data, size_t len, tel_DataType type) {
    uint32_t t = HAL_GetTick();

    _tel_WriteBytes(&t, 4);
    _tel_WriteBytes(&type, 1);
    _tel_WriteBytes(&topic, 1);
    _tel_WriteBytes(data, len);
}

void tel_WritePing(tel_Topic topic) {
    _tel_WriteMessage(topic, NULL, 0, Ping);
}

void tel_WriteInteger(tel_Topic topic, uint32_t i) {
    _tel_WriteMessage(topic, &i, 4, Ping);
}

void tel_WriteFloat(tel_Topic topic, double d) {
    _tel_WriteMessage(topic, &d, 8, Float);
}

void tel_WriteChar(tel_Topic topic, char c) {
    _tel_WriteMessage(topic, &c, 1, Char);
}

void tel_WriteBoolean(tel_Topic topic, bool b) {
    _tel_WriteMessage(topic, &b, 1, Boolean);
}

static uint8_t _tel_GetDataSize(tel_DataType type) {
    switch (type) {
        case Ping: return 0;
        case Integer: return 4;
        case Float: return 8;
        case Char: return 1;
        case Boolean: return 1;
        default: return -1;
    }
}

void tel_WriteArray(tel_Topic topic, const void* arr, uint8_t len, tel_DataType type) {
    _tel_WriteMessage(topic, (const char*)arr, len * _tel_GetDataSize(type), Array);
}

void tel_WriteString(tel_Topic topic, const char* str) {
    tel_WriteArray(topic, str, strlen(str), Char);
}
