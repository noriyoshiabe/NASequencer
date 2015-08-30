#pragma once

typedef struct _NAByteBuffer NAByteBuffer;

extern NAByteBuffer *NAByteBufferCreate(int allocationUnit);
extern void NAByteBufferDestroy(NAByteBuffer *self);
extern int NAByteBufferWriteString(NAByteBuffer *self, char *string);
extern int NAByteBufferWriteInteger(NAByteBuffer *self, int value);
extern int NAByteBufferWriteFloat(NAByteBuffer *self, float value);

extern int NAByteBufferReadString(NAByteBuffer *self, char **string);
extern int NAByteBufferReadInteger(NAByteBuffer *self, int *value);
extern int NAByteBufferReadFloat(NAByteBuffer *self, float *value);

extern void NAByteBufferSeekFirst(NAByteBuffer *self);
