#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void log_Debug(const char* format, ...) {
    va_list args;
    va_start(args, format);

    printf("[DEBUG] ", args);
    printf(format, args);
    puts("");

    va_end(args);
}
