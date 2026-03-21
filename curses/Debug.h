#pragma once

#include <stdarg.h>

static inline void Debug(const char *fmt, ...)
{
    va_list argList;
    va_start(argList, fmt);
    
    FILE *fp = fopen("debug.log", "a");
    if (fp != NULL) {
        vfprintf(fp, fmt, argList);
        fclose(fp);
    }

    va_end(argList);
}
