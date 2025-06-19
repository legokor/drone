#include "log/log.h"
#include "err/err.h"
#include "tel/tel.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

void log_init(void) {}

#define _log_BUF_SIZE 256

#define _log_TYPE_SIZE ((uint8_t) 3)
#define _log_PREFIX_SIZE (_log_TYPE_SIZE + sizeof("[] ") - 1)

static void _log_write(log_LogLevel level, const char* restrict format, va_list args) {
    const char LEVEL_STR[5][_log_TYPE_SIZE] = { "xxx", "DBG", "INF", "WRN", "ERR" };
    static char _log_buf[_log_BUF_SIZE];

    if (level != log_NONE) {
        _log_buf[0] = '[';
        memcpy(_log_buf + 1, LEVEL_STR[level], _log_TYPE_SIZE);
        _log_buf[_log_TYPE_SIZE + 1] = ']';
        _log_buf[_log_TYPE_SIZE + 2] = ' ';

        int cnt = vsnprintf(_log_buf + _log_PREFIX_SIZE, _log_BUF_SIZE - (_log_PREFIX_SIZE + 2), format, args);
        if (cnt < 0) {
            err_tryIgnorable(cnt >= 0, "Couldn't format log");
        }

        int idx = (uint8_t) cnt < _log_BUF_SIZE - (_log_PREFIX_SIZE + 2) ? cnt + _log_PREFIX_SIZE : _log_BUF_SIZE - 3;

        // copy null too
        memcpy(_log_buf + idx, "\r\n", 3);
    } else {
        err_tryIgnorable(vsnprintf(_log_buf, _log_BUF_SIZE, format, args) >= 0, "Couldn't format log");
    }

    tel_writeString(LOG_TOPIC, _log_buf);
}

#define _log_LOG_VARIANT(var)      \
    va_list args;                  \
    va_start(args, format);        \
    _log_write(var, format, args); \
    va_end(args);

void log_raw(const char* restrict format, ...) {
    _log_LOG_VARIANT(log_NONE);
}

void log_debug(const char* restrict format, ...) {
    _log_LOG_VARIANT(log_DEBUG);
}

void log_info(const char* restrict format, ...) {
    _log_LOG_VARIANT(log_INFO);
}

void log_warn(const char* restrict format, ...) {
    _log_LOG_VARIANT(log_WARN);
}

void log_error(const char* restrict format, ...) {
    _log_LOG_VARIANT(log_ERROR);
}

void log_write(log_LogLevel level, const char* restrict format, ...) {
    _log_LOG_VARIANT(level);
}
