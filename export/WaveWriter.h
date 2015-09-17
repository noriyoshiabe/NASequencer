#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct _WaveWriter WaveWriter;

extern WaveWriter *WaveWriterCreate();
extern void WaveWriterDestroy(WaveWriter *self);
extern bool WaveWriterOpenFile(WaveWriter *self, const char *filepath);
extern void WaveWriterAppendData(WaveWriter *self, int32_t *data, int count);

extern bool WaveWriterSerialize(WaveWriter *self);
