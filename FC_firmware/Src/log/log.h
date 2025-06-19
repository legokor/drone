#ifndef LOG_H
#define LOG_H

void log_init(void);

typedef enum log_LogLevel { log_NONE, log_DEBUG, log_INFO, log_WARN, log_ERROR } log_LogLevel;

void log_write(log_LogLevel level, const char* format, ...);

void log_raw(const char* format, ...);
void log_debug(const char* format, ...);
void log_info(const char* format, ...);
void log_warn(const char* format, ...);
void log_error(const char* format, ...);

#endif // LOG_H
