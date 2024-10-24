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

void errorLog(const char *funcName, const char *messageFormat, ...) {
    printf("Error in %s: ", funcName);

    va_list args;
    va_start(args, messageFormat);
    vprintf(messageFormat, args);
    va_end(args);

    printf("\n");
}