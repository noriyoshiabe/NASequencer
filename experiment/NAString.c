#include <NAString.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct __NAString {
    NATypeCtx ctx;
    char *buffer;
    int length;
};

NAString *NAStringAppend(NAString *self, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int length = vsnprintf(NULL, 0, format, ap);
    va_end(ap);

    va_start(ap, format);
    NAString *ret = NATypeLookup(self, NAString)->append(self, format, ap, length);
    va_end(ap);

    return ret;
}

char *NAStringCString(NAString *self)
{
    return NATypeLookup(self, NAString)->cstring(self);
}

static NAString *__NAStringAppend(NAString *self, const char *format, va_list ap, int length)
{
    self->buffer = (char *)realloc(self->buffer, self->length + length + 1);
    vsnprintf(self->buffer + self->length, length + 1, format, ap);
    NATypeResetHash(self);
    return self;
}

static char *__NAStringCString(NAString *self)
{
    return self->buffer;
}

static NAType *__NAStringInit(NAType *_self, ...)
{
    NAString *self = _self;
    const char *format;
    va_list ap;

    va_start(ap, _self);
    format = va_arg(ap, const char *);
    int length = vsnprintf(NULL, 0, format, ap);
    va_end(ap);

    self->buffer = (char *)malloc(length + 1);
    self->length = length;

    va_start(ap, _self);
    va_arg(ap, const char *);
    length = vsnprintf(self->buffer, length + 1, format, ap);
    va_end(ap);

    NATypeResetHash(self);
    return self;
}

static void __NAStringDestroy(NAType *_self)
{
    NAString *self = _self;
    free(self->buffer);
    printf("str released.\n");
}

static uint32_t __NAStringHash(NAType *_self)
{
    NAString *self = _self;
    uint32_t h = 0;
    for (int i = 0; i < self->length; ++i) {
        h = 31 * h + self->buffer[i];
    }
    return h;
}

static int __NAStringCompare(NAType *_self, NAType *_to)
{
    NAString *self = _self;
    NAString *to = _to;
    return strcmp(self->buffer, to->buffer);
}

static NAString *__NAStringToString(NAType *_self)
{
    return _self;
}

static NATypeVtbl typeVtbl = {
    __NAStringHash,
    __NATypeEqualTo,
    __NAStringCompare,
    __NAStringToString,
};

static NAStringVtbl stringVtbl = {
    __NAStringAppend,
    __NAStringCString,
};

static NAVtbl vtbl[] = {
    {&NATypeID, &typeVtbl},
    {&NAStringID, &stringVtbl},
    {NULL, NULL},
};

NAClass NAStringClass = {
    &NAStringID,
    "NAString",
    sizeof(NAString),
    __NAStringInit,
    __NAStringDestroy,
    vtbl,
};

int NAStringID;
