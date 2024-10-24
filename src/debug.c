#include "debug.h"

#include <stdarg.h>
#include <stdio.h>

void debugLog(const char *format, ...) {
    #ifdef DEBUG
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    #endif
}