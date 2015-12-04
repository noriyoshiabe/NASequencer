#pragma once

#include "Parser.h"
#include <stdbool.h>

typedef enum {
    ExporterErrorNoError,
    ExporterErrorUnsupportedFileType,
    ExporterErrorNoSoundSource,
    ExporterErrorCouldNotWriteFile,
} ExporterError;

typedef struct _Exporter Exporter;

typedef struct _ExporterObserverCallbacks {
    void (*onParseFinish)(void *receiver, ParseInfo *info);
    void (*onProgress)(void *receiver, int progress);
} ExporterObserverCallbacks;

extern Exporter *ExporterCreate(ExporterObserverCallbacks *callbacks, void *receiver);
extern void ExporterDestroy(Exporter *self);
extern ExporterError ExporterExport(Exporter *self, const char *filepath, const char *output);
