#include "Debug.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static DebugWindow *__debugWindow;

void DebugInit(DebugWindow *debugWindow)
{
    __debugWindow = debugWindow;

    FILE *fp = fopen("debug.log", "w");
    fclose(fp);
}

void Debug(const char *fmt, ...)
{
    va_list argList;
    va_start(argList, fmt);

    DebugWindowAppendLog(__debugWindow, fmt, argList);

    va_end(argList);
}

void DebugF(const char *fmt, ...)
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
