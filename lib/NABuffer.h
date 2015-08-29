#pragma once

#include <stdbool.h>

typedef struct _NABuffer NABuffer;

extern NABuffer *NABufferCreate(int allocationUnit);
extern void NABufferDestroy(NABuffer *self);
extern void NABufferWriteString(NABuffer *self, char *string);
extern void NABufferWriteInteger(NABuffer *self, int value);
extern void NABufferWriteFloat(NABuffer *self, float value);

extern bool NABufferReadString(NABuffer *self, char **string);
extern bool NABufferReadInteger(NABuffer *self, int *value);
extern bool NABufferReadFloat(NABuffer *self, float *value);

extern void NABufferSeekFirst(NABuffer *self);
