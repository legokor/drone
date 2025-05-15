#include "tel/tel.h"
#include <string.h>
#include "log/log.h"
#include "sys/sys.h"
#include "utils/utils.h"

#define MAX_SOURCE_COUNT 20

static tel_WriteFn _tel_Sources[MAX_SOURCE_COUNT] = { 0 };

void _tel_WriteUart(const void* data, size_t len) {
    uart_Transmit(&sys_UartInstance, data, len);
}

void tel_Init(void) {
    tel_AddSource(_tel_WriteUart);

    log_Debug("Telemetry initialized");
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
    // uint32_t t = utils_GetMsSinceStartup(); // TODO: uncomment this when the telemetry software is ready
    // _tel_WriteBytes(&t, 4);
    // _tel_WriteBytes(&type, 1);
    // _tel_WriteBytes(&topic, 1);
    _tel_WriteBytes(data, len);
}

void tel_WritePing(tel_Topic topic) {
    _tel_WriteMessage(topic, NULL, 0, tel_TYPE_PING);
}

void tel_WriteInteger(tel_Topic topic, uint32_t i) {
    _tel_WriteMessage(topic, &i, 4, tel_TYPE_INTEGER);
}

void tel_WriteFloat(tel_Topic topic, double d) {
    _tel_WriteMessage(topic, &d, 8, tel_TYPE_FLOAT);
}

void tel_WriteChar(tel_Topic topic, char c) {
    _tel_WriteMessage(topic, &c, 1, tel_TYPE_CHAR);
}

void tel_WriteBoolean(tel_Topic topic, bool b) {
    _tel_WriteMessage(topic, &b, 1, tel_TYPE_BOOLEAN);
}

static uint8_t _tel_GetDataSize(tel_DataType type) {
    switch (type) {
        case tel_TYPE_PING: return 0;
        case tel_TYPE_INTEGER: return 4;
        case tel_TYPE_FLOAT: return 8;
        case tel_TYPE_CHAR: return 1;
        case tel_TYPE_BOOLEAN: return 1;
        default: return -1;
    }
}

void tel_WriteArray(tel_Topic topic, const void* arr, uint8_t len, tel_DataType type) {
    _tel_WriteMessage(topic, arr, len * _tel_GetDataSize(type), tel_TYPE_ARRAY);
}

void tel_WriteString(tel_Topic topic, const char* str) {
    tel_WriteArray(topic, str, strlen(str), tel_TYPE_CHAR);
}
