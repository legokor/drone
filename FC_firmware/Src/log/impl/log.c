#include "log.h"
#include "tel.h"

#include <stdarg.h>
#include <stdio.h>

void log_Init(tel_Tel* tel);

void log_Debug(const char* format, ...) {
    va_list args;
    va_start(args, format);

    // TODO: use tel
    printf("[DEBUG] ");
    printf(format, args);
    printf("\n");

    va_end(args);
}
