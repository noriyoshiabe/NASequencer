#pragma once

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

extern CLI *CLICreate(const char *filepath, const char *soundSource);
extern void CLIDestroy(CLI *self);
extern CLIError CLIRunShell(CLI *self);
extern void CLISigInt(CLI *self);
extern CLIError CLIExport(CLI *self, const char *output);
