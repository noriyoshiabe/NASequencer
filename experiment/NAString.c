#include <NAString.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct __NAString {
    NAType _;
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
    return self;
}

static char *__NAStringCString(NAString *self)
{
    return self->buffer;
}

static void *__NAStringInit(void *_self, ...)
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

    return self;
}

static void __NAStringDestroy(void *_self)
{
    NAString *self = _self;
    free(self->buffer);
}

static uint32_t __NAStringHash(const void *_self)
{
    const NAString *self = _self;
    uint32_t h = 0;
    for (int i = 0; i < self->length; ++i) {
        h = 31 * h + self->buffer[i];
    }
    return h;
}

static int __NAStringCompare(const void *_self, const void *_to)
{
    const NAString *self = _self;
    const NAString *to = _to;
    return strcmp(self->buffer, to->buffer);
}

static NATypeVtbl typeVtbl = {
    __NAStringInit,
    __NAStringDestroy,
    __NAStringHash,
    NATypeEqualDefault,
    __NAStringCompare,
};

static NAStringVtbl stringVtbl = {
    __NAStringAppend,
    __NAStringCString,
};

static NAVtblEntry vEntries[] = {
    {NATypeID, &typeVtbl},
    {NAStringID, &stringVtbl},
    {NULL, NULL},
};

NAClass NAStringClass = {
    NAStringID,
    sizeof(NAString),
    vEntries,
};

char NAStringID[] = "NAString";
