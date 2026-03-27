#pragma once

#include "DebugWindow.h"
#include <stdarg.h>

extern void DebugInit(DebugWindow *debugWindow);
extern void Debug(const char *fmt, ...);

#define __Trace__ Debug("-- %s - %d - %s\n", __FILE__, __LINE__, __FUNCTION__);
#define __Dump__I(i) Debug("-- [%s:%d] %s - %d - %s\n", #i, (int)i, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__L(i) Debug("-- [%s:%ld] %s - %d - %s\n", #i, (int64_t)i, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__H(h) Debug("-- [%s:%08X] %s - %d - %s\n", #h, (int)h, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__S(s) Debug("-- [%s:%s] %s - %d - %s\n", #s, (char *)s, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__C(c) Debug("-- [%s:%c] %s - %d - %s\n", #c, (char)c, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__F(f) Debug("-- [%s:%.2f] %s - %d - %s\n", #f, (float)f, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__D(d) Debug("-- [%s:%.2f] %s - %d - %s\n", #d, (double)f, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__P(p) Debug("-- [%s:%08X] %s - %d - %s\n", #p, (uintptr_t)p, __FILE__, __LINE__, __FUNCTION__);
