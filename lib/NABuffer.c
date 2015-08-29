#include "NABuffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct _NABuffer {
    uint8_t *buffer;
    int writeCursor;
    int readCursor;
    int allocationUnit;
    int capacity;
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
    while (sizeNeeded > self->capacity - self->writeCursor) {
        self->capacity += self->allocationUnit;
    }

    self->buffer = realloc(self->buffer, self->capacity);
}

void NABufferWriteString(NABuffer *self, char *string)
{
    int length = strlen(string) + 1;
    if (self->capacity - self->writeCursor < length ) {
        NABufferExtend(self, length);
    }

    strcpy((char *)(self->buffer + self->writeCursor), string);
    self->writeCursor += length;
}

void NABufferWriteInteger(NABuffer *self, int value)
{
    if (self->capacity - self->writeCursor < sizeof(int)) {
        NABufferExtend(self, sizeof(int));
    }

    *((int *)(self->buffer + self->writeCursor)) = value;
    self->writeCursor += sizeof(int);
}

void NABufferWriteFloat(NABuffer *self, float value)
{
    if (self->capacity - self->writeCursor < sizeof(float)) {
        NABufferExtend(self, sizeof(float));
    }

    *((float *)(self->buffer + self->writeCursor)) = value;
    self->writeCursor += sizeof(float);
}

bool NABufferReadString(NABuffer *self, char **string)
{
    if (self->writeCursor <= self->readCursor) {
        return false;
    }

    *string = (char *)(self->buffer + self->readCursor);
    self->readCursor += strlen((char *)(self->buffer + self->readCursor));
    return true;
}

bool NABufferReadInteger(NABuffer *self, int *value)
{
    if (self->writeCursor <= self->readCursor) {
        return false;
    }

    *value = *((int *)(self->buffer + self->readCursor));
    self->readCursor += sizeof(int);
    return true;
}

bool NABufferReadFloat(NABuffer *self, float *value)
{
    if (self->writeCursor <= self->readCursor) {
        return false;
    }

    *value = *((float *)(self->buffer + self->readCursor));
    self->readCursor += sizeof(float);
    return true;
}

void NABufferSeekFirst(NABuffer *self)
{
    self->readCursor = 0;
}
