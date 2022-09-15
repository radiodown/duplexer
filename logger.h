#ifndef LOGGER_H
#define LOGGER_H

#define LOG_LEVEL 7

#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG_TRACE 4

#define logger(level, fmt, ...) logger_(level , __FUNCTION__, fmt, ##__VA_ARGS__)

void dump_hex(const void* data, int size);
void logger_(int level, const char* funcname, void* format, ...);

#endif