#ifndef LOG_H
#define LOG_H

#include "tel/tel.h"

void log_Init(tel_Tel* tel);

void log_Debug(const char* format, ...);

#endif // LOG_H
