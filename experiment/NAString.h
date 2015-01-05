#pragma once

#include <NAType.h>

#include <stdarg.h>

typedef struct __NAString NAString;

extern NAString *NAStringAppend(NAString *self, const char *format, ...);
extern char *NAStringCString(NAString *self);

typedef struct __NAStringVtbl {
    NAString *(*append)(NAString *self, const char *format, va_list ap, int length);
    char *(*cstring)(NAString *self);
} NAStringVtbl;

extern NAClass NAStringClass;
extern int NAStringID;
