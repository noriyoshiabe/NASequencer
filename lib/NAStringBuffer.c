#include "NAStringBuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/param.h>

struct _NAStringBuffer {
    char *buffer;
    int length;
    int allocationUnit;
    int capacity;
};

NAStringBuffer *NAStringBufferCreate(int allocationUnit)
{
    NAStringBuffer *self = calloc(1, sizeof(NAStringBuffer));
    self->buffer = malloc(allocationUnit);
    self->allocationUnit = allocationUnit;
    self->capacity = allocationUnit;
    return self;
}

void NAStringBufferDestroy(NAStringBuffer *self)
{
    if (self->buffer) {
        free(self->buffer);
    }
    free(self);
}

static void NAStringBufferExtend(NAStringBuffer *self, int sizeNeeded)
{
    while (sizeNeeded > self->capacity - self->length) {
        self->capacity += self->allocationUnit;
    }

    self->buffer = realloc(self->buffer, self->capacity);
}

int NAStringBufferAppendString(NAStringBuffer *self, const char *string)
{
    int length = strlen(string);
    if (self->capacity - self->length < length + 1) {
        NAStringBufferExtend(self, length + 1);
    }

    memcpy((self->buffer + self->length), string, length);
    self->length += length;
    return length;
}

int NAStringBufferAppendNString(NAStringBuffer *self, const char *string, int length)
{
    if (self->capacity - self->length < length + 1) {
        NAStringBufferExtend(self, length + 1);
    }

    memcpy((self->buffer + self->length), string, length);
    self->length += length;
    return length;
}

extern int NAStringBufferAppendFormat(NAStringBuffer *self, const char *format, ...)
{
    va_list argList;
    va_start(argList, format);
    int length = vsnprintf(NULL, 0, format, argList);
    va_end(argList);

    if (self->capacity - self->length < length + 1) {
        NAStringBufferExtend(self, length + 1);
    }

    va_start(argList, format);
    vsnprintf(self->buffer + self->length, length + 1, format, argList);
    self->length += length;
    va_end(argList);

    return length;
}

int NAStringBufferAppendChar(NAStringBuffer *self, char c)
{
    if (self->capacity - self->length < 2) {
        NAStringBufferExtend(self, 2);
    }

    *(self->buffer + self->length) = c;
    ++self->length;
    return 1;
}

int NAStringBufferGetCString(NAStringBuffer *self, char *buffer, int size)
{
    int length = MIN(self->length, size - 1);
    memcpy(buffer, self->buffer, length);
    buffer[length] = '\0';
    return length;
}

int NAStringBufferGetLength(NAStringBuffer *self)
{
    return self->length;
}

char *NAStringBufferRetriveCString(NAStringBuffer *self)
{
    self->buffer[self->length] = '\0';
    char *ret = self->buffer;
    self->buffer = NULL;
    self->length = 0;
    self->capacity = 0;
    return ret;
}

void NAStringBufferClear(NAStringBuffer *self)
{
    self->length = 0;
}
