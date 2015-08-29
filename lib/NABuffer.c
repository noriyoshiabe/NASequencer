#include "NABuffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct _NABuffer {
    int writeCursor;
    int readCursor;
    uint8_t *buffer;

    int allocationUnit;
    int capacity;
    int size;
};

NABuffer *NABufferCreate(int allocationUnit)
{
    NABuffer *self = calloc(1, sizeof(NABuffer));
    self->buffer = malloc(allocationUnit);
    self->allocationUnit = allocationUnit;
    self->capacity = allocationUnit;
    return self;
}

void NABufferDestroy(NABuffer *self)
{
    free(self->buffer);
    free(self);
}

static void NABufferExtend(NABuffer *self, int sizeNeeded)
{
    while (sizeNeeded > self->capacity - self->size) {
        self->capacity += self->allocationUnit;
    }

    self->buffer = realloc(self->buffer, self->capacity);
}

void NABufferWriteString(NABuffer *self, char *string)
{
    int length = strlen(string) + 1;
    if (self->capacity - self->size < length ) {
        NABufferExtend(self, length);
    }

    strcpy((char *)(self->buffer + self->writeCursor), string);
    self->writeCursor += length;
}

void NABufferWriteInteger(NABuffer *self, int value)
{
    if (self->capacity - self->size < sizeof(int)) {
        NABufferExtend(self, sizeof(int));
    }

    *((int *)(self->buffer + self->writeCursor)) = value;
    self->writeCursor += sizeof(int);
}

void NABufferWriteFloat(NABuffer *self, float value)
{
    if (self->capacity - self->size < sizeof(float)) {
        NABufferExtend(self, sizeof(float));
    }

    *((float *)(self->buffer + self->writeCursor)) = value;
    self->writeCursor += sizeof(float);
}

char *NABufferReadString(NABuffer *self)
{
    char *ret = (char *)(self->buffer + self->readCursor);
    self->readCursor += strlen((char *)(self->buffer + self->readCursor));
    return ret;
}

int NABufferReadInteger(NABuffer *self)
{
    int ret = *((int *)(self->buffer + self->readCursor));
    self->readCursor += sizeof(int);
    return ret;
}

float NABufferReadFloat(NABuffer *self)
{
    float ret = *((float *)(self->buffer + self->readCursor));
    self->readCursor += sizeof(float);
    return ret;
}

void NABufferSeekFirst(NABuffer *self)
{
    self->readCursor = 0;
    self->writeCursor = 0;
}
