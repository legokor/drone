#ifndef LOG_H
#define LOG_H

#include "tel/tel.h"

void log_Init(void);

typedef enum log_LogLevel { None, Debug, Info, Warn, Error } log_LogLevel;

void log_Write(log_LogLevel level, const char* format, ...);

void log_Debug(const char* format, ...);
void log_Info(const char* format, ...);
void log_Warn(const char* format, ...);
void log_Error(const char* format, ...);

#endif // LOG_H
