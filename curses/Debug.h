#pragma once

#include "DebugWindow.h"
#include <stdarg.h>

extern void DebugInit(DebugWindow *debugWindow);
extern void Debug(const char *fmt, ...);
extern void DebugF(const char *fmt, ...);
