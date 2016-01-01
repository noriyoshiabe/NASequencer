#pragma once

#include <stdio.h>

#ifdef DEBUG

#define __Trace__    printf("-- %s - %d - %s\n", __FILE__, __LINE__, __FUNCTION__);
#define __Dump__I(i) printf("-- [%s:%d] %s - %d - %s\n", #i, (int)i, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__H(h) printf("-- [%s:%08X] %s - %d - %s\n", #h, (int)h, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__S(s) printf("-- [%s:%s] %s - %d - %s\n", #s, (char *)s, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__C(c) printf("-- [%s:%c] %s - %d - %s\n", #c, (char)c, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__F(f) printf("-- [%s:%.2f] %s - %d - %s\n", #f, (float)f, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__D(d) printf("-- [%s:%.2f] %s - %d - %s\n", #d, (double)f, __FILE__, __LINE__, __FUNCTION__);
#define __Dump__P(p) printf("-- [%s:%08X] %s - %d - %s\n", #p, (int)p, __FILE__, __LINE__, __FUNCTION__);

#define __IsDebug__  (NALogLevelDebug <= NALogLevel)
#define __IsTrace__  (NALogLevelTrace <= NALogLevel)

#define __LogDebug__(...) __IsDebug__ ? fprintf(NALogStream, __VA_ARGS__) : 0
#define __LogTrace__(...) __IsTrace__ ? fprintf(NALogStream, __VA_ARGS__) : 0

extern const int NALogLevelSilent;
extern const int NALogLevelDebug;
extern const int NALogLevelTrace;

extern int NALogLevel;
extern FILE *NALogStream;

#else

#define __Trace__   
#define __Dump__I(i)
#define __Dump__H(h)
#define __Dump__S(s)
#define __Dump__C(c)
#define __Dump__F(f)
#define __Dump__D(d)
#define __Dump__P(p)

#define __IsDebug__ 0
#define __IsTrace__ 0

#define __LogTrace__(...)
#define __LogDebug__(...)

#endif
