#include "log/log.h"
#include "tel/tel.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

void log_Init(void) {}

#define WRITE_BUF_SIZE 256

void log_Write(log_LogLevel level, const char* format, ...) {
    const char LEVEL_STR[5][3] = { "xxx", "DBG", "INF", "WRN", "ERR" };

    va_list args;
    va_start(args, format);

    char buf[WRITE_BUF_SIZE];

    if (level != None) {
        buf[0] = '[';
        strncpy(buf + 1, LEVEL_STR[level], 3);
        buf[4] = ']';
    }

    int offs = level == None ? 0 : 6;
    vsnprintf(buf + offs, WRITE_BUF_SIZE - offs, format, args);

    tel_WriteString(LOG_TOPIC, buf);

    va_end(args);
}

void log_Debug(const char* format, ...) {
    // va_list args;
    // va_start(args, format);
    // log_Write(Debug, format, args);
    // va_end(args);
}

void log_Info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_Write(Info, format, args);
    va_end(args);
}

void log_Warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_Write(Warn, format, args);
    va_end(args);
}

void log_Error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_Write(Error, format, args);
    va_end(args);
}
