#include "NAByteBuffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#undef NAByteBufferReadData

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

int NAByteBufferWriteData(NAByteBuffer *self, void *data, int length)
{
    if (self->capacity - self->writeCursor < length ) {
        NAByteBufferExtend(self, length);
    }

    memcpy(self->buffer + self->writeCursor, data, length);
    self->writeCursor += length;
    return length;
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

int NAByteBufferReadData(NAByteBuffer *self, void **data, int length)
{
    if (self->writeCursor <= self->readCursor) {
        return 0;
    }

    *data = (void *)(self->buffer + self->readCursor);
    length = MIN(length, self->writeCursor - self->readCursor);
    self->readCursor += length;
    return length;
}

int NAByteBufferDataLength(NAByteBuffer *self)
{
    return self->writeCursor - self->readCursor;
}

void NAByteBufferSeekFirst(NAByteBuffer *self)
{
    self->readCursor = 0;
}
