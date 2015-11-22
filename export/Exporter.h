#pragma once

#include "BuildInformation.h"
#include <stdbool.h>

typedef enum {
    ExporterErrorNoError,
    ExporterErrorUnsupportedFileType,
    ExporterErrorNoSoundSource,
    ExporterErrorParseFailed,
    ExporterErrorCouldNotWriteFile,
} ExporterError;

typedef struct _Exporter Exporter;

typedef struct _ExporterObserverCallbacks {
    void (*onParseFinish)(void *receiver, BuildInformation *info);
    void (*onProgress)(void *receiver, int progress);
} ExporterObserverCallbacks;

extern Exporter *ExporterCreate(ExporterObserverCallbacks *callbacks, void *receiver);
extern void ExporterDestroy(Exporter *self);
extern ExporterError ExporterExport(Exporter *self, const char *filepath, const char *output);
