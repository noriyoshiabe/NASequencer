#pragma once

#include <stdbool.h>

typedef struct _WaveWriter WaveWriter;

extern WaveWriter *WaveWriterCreate();
extern void WaveWriterDestroy(WaveWriter *self);
extern bool WaveWriterOpenFile(WaveWriter *self, const char *filepath);

extern bool WaveWriterSerialize(WaveWriter *self);
