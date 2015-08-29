#pragma once

typedef struct _NABuffer NABuffer;

extern NABuffer *NABufferCreate(int allocationUnit);
extern void NABufferDestroy(NABuffer *self);
extern void NABufferWriteString(NABuffer *self, char *string);
extern void NABufferWriteInteger(NABuffer *self, int value);
extern void NABufferWriteFloat(NABuffer *self, float value);

extern char *NABufferReadString(NABuffer *self);
extern int NABufferReadInteger(NABuffer *self);
extern float NABufferReadFloat(NABuffer *self);

extern void NABufferSeekFirst(NABuffer *self);
