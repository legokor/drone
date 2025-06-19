#include "log/log.h"
#include "err/err.h"
#include "tel/tel.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

void log_init(void) {}

#define _log_WRITE_BUF_SIZE 256

static char _log_buf[_log_WRITE_BUF_SIZE];

static void _log_write(log_LogLevel level, const char* format, va_list args) {
    const char LEVEL_STR[5][3] = { "xxx", "DBG", "INF", "WRN", "ERR" };

    if (level != log_NONE) {
        _log_buf[0] = '[';
        strncpy(_log_buf + 1, LEVEL_STR[level], 3);
        _log_buf[4] = ']';
    }

    if (level == log_NONE) {
        vsnprintf(_log_buf, _log_WRITE_BUF_SIZE, format, args);
    } else {
        int ret = vsnprintf(_log_buf + 5, _log_WRITE_BUF_SIZE - 7, format, args);
        int nullTermIndex = ret < _log_WRITE_BUF_SIZE - 7 ? ret + 5 : _log_WRITE_BUF_SIZE - 3;

        _log_buf[nullTermIndex] = '\r';
        _log_buf[nullTermIndex + 1] = '\n';
        _log_buf[nullTermIndex + 2] = '\0';
    }

    tel_writeString(LOG_TOPIC, _log_buf);
}

void log_raw(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_NONE, format, args);
    va_end(args);
}

void log_debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_DEBUG, format, args);
    va_end(args);
}

void log_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_INFO, format, args);
    va_end(args);
}

void log_warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_WARN, format, args);
    va_end(args);
}

void log_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_ERROR, format, args);
    va_end(args);
}

void log_write(log_LogLevel level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(level, format, args);
    va_end(args);
}
