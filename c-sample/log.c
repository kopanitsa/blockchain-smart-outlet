//
// Created by Okada, Takahiro on 2017/04/03.
//

#include "log.h"

void debug_msg(const char *file, const char *function, int line, const char *fmt, ...) {
    printf("[%s %s %d] ", file, function, line);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);

    printf("\n");
}

void err_msg(const char *file, const char *function, int line, const char *fmt, ...) {
    printf("[%s %s %d] ", file, function, line);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    printf("\n");
}

