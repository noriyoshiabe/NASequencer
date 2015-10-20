#pragma once

#include "Sequence.h"
#include <stdbool.h>

typedef struct _Exporter Exporter;
typedef void (*ExporterProgressCallback)(void *receiver, int progress);

extern Exporter *ExporterCreate(Sequence *sequence);
extern void ExporterSetProgressCallback(Exporter *self, ExporterProgressCallback callback, void *receiver);
extern void ExporterDestroy(Exporter *self);
extern bool ExporterWriteToSMF(Exporter *self, const char *filepath);
extern bool ExporterWriteToWave(Exporter *self, const char *filepath);
extern bool ExporterWriteToAAC(Exporter *self, const char *filepath);
