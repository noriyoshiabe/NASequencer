#include <stdint.h>
#include <stdbool.h>

#include "AudioSample.h"

typedef struct _AACWriter AACWriter;

extern AACWriter *AACWriterCreate(double sampleRate);
extern void AACWriterDestroy(AACWriter *self);
extern bool AACWriterOpenFile(AACWriter *self, const char *filepath);
extern void AACWriterAppendData(AACWriter *self, int32_t *data, int count);

extern bool AACWriterSerialize(AACWriter *self);
