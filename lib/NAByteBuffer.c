#include "NAByteBuffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct _NAByteBuffer {
    uint8_t *buffer;
    int writeCursor;
    int readCursor;
    int allocationUnit;
    int capacity;
};

NAByteBuffer *NAByteBufferCreate(int allocationUnit)
{
    NAByteBuffer *self = calloc(1, sizeof(NAByteBuffer));
    self->buffer = malloc(allocationUnit);
    self->allocationUnit = allocationUnit;
    self->capacity = allocationUnit;
    return self;
}

void NAByteBufferDestroy(NAByteBuffer *self)
{
    free(self->buffer);
    free(self);
}

static void NAByteBufferExtend(NAByteBuffer *self, int sizeNeeded)
{
    while (sizeNeeded > self->capacity - self->writeCursor) {
        self->capacity += self->allocationUnit;
    }

    self->buffer = realloc(self->buffer, self->capacity);
}

int NAByteBufferWriteString(NAByteBuffer *self, char *string)
{
    int length = strlen(string) + 1;
    if (self->capacity - self->writeCursor < length ) {
        NAByteBufferExtend(self, length);
    }

    strcpy((char *)(self->buffer + self->writeCursor), string);
    self->writeCursor += length;
    return length;
}

int NAByteBufferWriteInteger(NAByteBuffer *self, int value)
{
    if (self->capacity - self->writeCursor < sizeof(int)) {
        NAByteBufferExtend(self, sizeof(int));
    }

    *((int *)(self->buffer + self->writeCursor)) = value;
    self->writeCursor += sizeof(int);
    return sizeof(int);
}

int NAByteBufferWriteFloat(NAByteBuffer *self, float value)
{
    if (self->capacity - self->writeCursor < sizeof(float)) {
        NAByteBufferExtend(self, sizeof(float));
    }

    *((float *)(self->buffer + self->writeCursor)) = value;
    self->writeCursor += sizeof(float);
    return sizeof(float);
}

int NAByteBufferReadString(NAByteBuffer *self, char **string)
{
    if (self->writeCursor <= self->readCursor) {
        return 0;
    }

    *string = (char *)(self->buffer + self->readCursor);
    int length = strlen(*string) + 1;
    self->readCursor += length;
    return length;
}

int NAByteBufferReadInteger(NAByteBuffer *self, int *value)
{
    if (self->writeCursor <= self->readCursor) {
        return 0;
    }

    *value = *((int *)(self->buffer + self->readCursor));
    self->readCursor += sizeof(int);
    return sizeof(int);
}

int NAByteBufferReadFloat(NAByteBuffer *self, float *value)
{
    if (self->writeCursor <= self->readCursor) {
        return 0;
    }

    *value = *((float *)(self->buffer + self->readCursor));
    self->readCursor += sizeof(float);
    return sizeof(float);
}

void NAByteBufferSeekFirst(NAByteBuffer *self)
{
    self->readCursor = 0;
}
