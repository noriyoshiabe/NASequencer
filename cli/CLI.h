#pragma once

#include "NAMidi.h"
#include "PianoRollView.h"
#include "EventListView.h"

#include <stdbool.h>

typedef enum {
    CLIErrorNoError,

    CLIErrorExportWithNoInputFile,
    CLIErrorExportWithUnsupportedFileType,
    CLIErrorExportWithParseFailed,
    CLIErrorExportWithNoSoundSource,
    CLIErrorExportWithSoundSourceLoadFailed,
    CLIErrorExportWithCannotWriteToOutputFile,
} CLIError;

typedef struct _CLI CLI;

extern CLI *CLICreate(const char *filepath, const char **soundSources);
extern void CLIDestroy(CLI *self);
extern CLIError CLIRunShell(CLI *self);
extern void CLISigInt(CLI *self);
extern void CLIExit(CLI *self);
extern CLIError CLIExport(CLI *self, const char *output);

extern NAMidi *CLIGetNAMidi(CLI *self);
extern PianoRollView *CLIGetPianoRollView(CLI *self);
extern EventListView *CLIGetEventListView(CLI *self);
extern void CLISetActiveView(CLI *self, void *view);
