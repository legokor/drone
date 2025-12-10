#include "log/log.h"
#include "err/err.h"
#include "tel/tel.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#define _log_BUF_SIZE 1024

#define _log_TYPE_SIZE 3
#define _log_PREFIX_SIZE (_log_TYPE_SIZE + sizeof("[] ") - 1)

#define _log_END "\r\n"

const char _log_LEVEL_STR[5][_log_TYPE_SIZE + 1] = { "xxx", "DBG", "INF", "WRN", "ERR" };

static char _log_buf[_log_BUF_SIZE];

// TODO: should log be a telemetry endpoint instead of it writing to telemetry
void log_init(void) {
    err_assert(_log_BUF_SIZE >= _log_TYPE_SIZE + sizeof(_log_END));
}

static void _log_write(log_LogLevel level, const char* restrict format, va_list args) {
    err_assert(log_NONE <= level && level <= log_ERROR);

    char* buf = _log_buf;
    size_t buf_size = _log_BUF_SIZE;
    if (level != log_NONE) {
        _log_buf[0] = '[';
        memcpy(_log_buf + 1, _log_LEVEL_STR[level], _log_TYPE_SIZE);
        _log_buf[_log_TYPE_SIZE + 1] = ']';
        _log_buf[_log_TYPE_SIZE + 2] = ' ';

        buf += _log_PREFIX_SIZE;
        buf_size -= _log_PREFIX_SIZE;

        // TODO: don't necessarely write \r\n
    }

    int format_ret = vsnprintf( //
        buf,
        // vsnprintf inserts \0, we don't need to count it
        buf_size - (sizeof(_log_END) - 1),
        format, //
        args    //
    );
    if (format_ret < 0) {
        err_tryIgnorable(format_ret >= 0, "Couldn't format log");
        return;
    }

    // truncate if needed
    uint32_t idx = utils_minI32(format_ret, buf_size - sizeof(_log_END));

    // copy null too
    memcpy(buf + idx, _log_END, sizeof(_log_END));

    tel_writeString(config_LOG_TOPIC, _log_buf);
}

void log_raw(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_NONE, format, args);
    va_end(args);
}

void log_debug(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_DEBUG, format, args);
    va_end(args);
}

void log_info(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_INFO, format, args);
    va_end(args);
}

void log_warn(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_WARN, format, args);
    va_end(args);
}

void log_error(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(log_ERROR, format, args);
    va_end(args);
}

void log_write(log_LogLevel level, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    _log_write(level, format, args);
    va_end(args);
}
