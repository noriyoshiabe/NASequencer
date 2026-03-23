#include "Debug.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static DebugWindow *__debugWindow = NULL;

void DebugInit(DebugWindow *debugWindow)
{
    __debugWindow = debugWindow;
}

void Debug(const char *fmt, ...)
{
    va_list argList;
    va_start(argList, fmt);

#if 1
    if (__debugWindow) {
        DebugWindowAppendLog(__debugWindow, fmt, argList);
    }
#else
    FILE *fp = fopen("debug.log", "a");
    if (fp != NULL) {
        vfprintf(fp, fmt, argList);
        fclose(fp);
    }
#endif

    va_end(argList);
}
