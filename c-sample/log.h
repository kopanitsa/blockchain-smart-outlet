//
// Created by Okada, Takahiro on 2017/04/03.
//

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

void debug_msg(const char *file, const char *function, int line, const char *fmt, ...);
void err_msg(const char *file, const char *function, int line, const char *fmt, ...);

#define LOGFILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG(fmt, ...)   debug_msg(LOGFILE, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) err_msg(LOGFILE, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#endif //LOG_H

