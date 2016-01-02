#pragma once

typedef struct _NAStringBuffer NAStringBuffer;

extern NAStringBuffer *NAStringBufferCreate(int allocationUnit);
extern void NAStringBufferDestroy(NAStringBuffer *self);
extern int NAStringBufferAppendString(NAStringBuffer *self, const char *string);
extern int NAStringBufferAppendNString(NAStringBuffer *self, const char *string, int length);
extern int NAStringBufferAppendFormat(NAStringBuffer *self, const char *format, ...);
extern int NAStringBufferAppendChar(NAStringBuffer *self, char c);
extern int NAStringBufferGetCString(NAStringBuffer *self, char *buffer, int size);
extern int NAStringBufferGetLength(NAStringBuffer *self);
extern char *NAStringBufferRetriveCString(NAStringBuffer *self);
extern void NAStringBufferClear(NAStringBuffer *self);
