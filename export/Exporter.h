#pragma once

#include "Sequence.h"
#include <stdbool.h>

typedef struct _Exporter Exporter;

extern Exporter *ExporterCreate(Sequence *sequence);
extern void ExporterDestroy(Exporter *self);
extern bool ExporterWriteToSMF(Exporter *self, const char *filepath);
extern bool ExporterWriteToWave(Exporter *self, const char *filepath);
