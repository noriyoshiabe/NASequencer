#pragma once

#include "Sequence.h"
#include <stdbool.h>

typedef struct _Exporter Exporter;

extern Exporter *ExporterCreate(const char *filepath, const char *soundSource);
extern void ExporterDestroy(Exporter *self);
extern bool ExporterWriteToSMF(Exporter *self, const char *filepath);
