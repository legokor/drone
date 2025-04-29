#include "log/log.h"
#include "tel/tel.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

void log_Init(void) {}

#define LOG_WRITE_BUF_SIZE 256

char _log_buf[LOG_WRITE_BUF_SIZE];

void _log_Write(log_LogLevel level, const char* format, va_list args) {
    const char LEVEL_STR[5][3] = { "xxx", "DBG", "INF", "WRN", "ERR" };

    if (level != log_NONE) {
        _log_buf[0] = '[';
        strncpy(_log_buf + 1, LEVEL_STR[level], 3);
        _log_buf[4] = ']';
    }

    int offs = level == log_NONE ? 0 : 5;
    vsnprintf(_log_buf + offs, LOG_WRITE_BUF_SIZE - offs, format, args);

    tel_WriteString(LOG_TOPIC, _log_buf);
}

void log_Raw(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_Write(log_NONE, format, args);
    va_end(args);
}

void log_Debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_Write(log_DEBUG, format, args);
    va_end(args);
}

void log_Info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_Write(log_INFO, format, args);
    va_end(args);
}

void log_Warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_Write(log_WARN, format, args);
    va_end(args);
}

void log_Error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_Write(log_ERROR, format, args);
    va_end(args);
}

void log_Write(log_LogLevel level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_Write(level, format, args);
    va_end(args);
}
