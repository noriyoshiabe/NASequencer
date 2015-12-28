#include "NAStringBuffer.h"

#include <stdlib.h>
#include <string.h>
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
    free(self->buffer);
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

void NAStringBufferClear(NAStringBuffer *self)
{
    self->length = 0;
}
