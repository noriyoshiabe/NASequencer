#include "CLI.h"
#include "NAMidi.h"
#include "Parser.h"
#include "Command.h"
#include "Exporter.h"
#include "NAUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <setjmp.h>
#include <readline/readline.h>
#include <readline/history.h>

#define PROMPT "namidi> "
#define MAX_HISTORY 100

struct _CLI {
    const char *filepath;
    const char *soundSource;
    NAMidi *namidi;
    MidiSourceManager *manager;
    sigjmp_buf jmpBuf;
};

static NAMidiObserverCallbacks CLINAMidiObserverCallbacks;
static MidiSourceManagerObserverCallbacks CLIMidiSourceManagerObserverCallbacks;

CLI *CLICreate(const char *filepath, const char *soundSource)
{
    CLI *self = calloc(1, sizeof(CLI));
    self->filepath = filepath;
    self->soundSource = soundSource;
    self->namidi = NAMidiCreate();
    self->manager = MidiSourceManagerSharedInstance();
    NAMidiAddObserver(self->namidi, self, &CLINAMidiObserverCallbacks);
    MidiSourceManagerAddObserver(self->manager, self, &CLIMidiSourceManagerObserverCallbacks);
    return self;
}

void CLIDestroy(CLI *self)
{
    MidiSourceManagerRemoveObserver(self->manager, self);
    NAMidiRemoveObserver(self->namidi, self);
    NAMidiDestroy(self->namidi);
    free(self);
}

CLIError CLIRunShell(CLI *self)
{
    char historyFile[PATH_MAX];
    char *line = NULL;
    int historyCount = 0;

    if (self->soundSource) {
        MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(self->manager, self->soundSource);
    }

    if (self->filepath) {
        NAMidiSetWatchEnable(self->namidi, true);
        NAMidiParse(self->namidi, self->filepath);
    }

    sprintf(historyFile, "%s/.namidi_history", getenv("HOME"));
    read_history(historyFile);

    while (sigsetjmp(self->jmpBuf, 1));

    while ((line = readline(PROMPT))) {
        Command *cmd = CommandParse(line);
        CommandExecute(cmd, self->namidi);

        if ('\0' != line[0]) {
            add_history(line);

            if (MAX_HISTORY < ++historyCount) {
                free(remove_history(0));
            }
        }

        free(line);
    }

    printf("\n");

    write_history(historyFile);
    clear_history();

    return CLIErrorNoError;
}

void CLISigInt(CLI *self)
{
    printf("\n");
    siglongjmp(self->jmpBuf, 1);
}

static CLIError CLIExportSMF(CLI *self, Sequence *sequence, const char *output);
static CLIError CLIExportWAV(CLI *self, Sequence *sequence, const char *output);
static CLIError CLIExportMP3(CLI *self, Sequence *sequence, const char *output);

CLIError CLIExport(CLI *self, const char *output)
{
    if (!self->filepath) {
        return CLIErrorExportWithNoInputFile;
    }

    ParseResult result = {};
    if (!ParserParseFile(self->filepath, &result)) {
        fprintf(stderr, "%s %s - %d:%d\n", ParseErrorKind2String(result.error.kind), result.error.location.filepath, result.error.location.line, result.error.location.column);
        return CLIErrorExportWithParseFailed;
    }

    const struct {
        const char *ext;
        CLIError (*function)(CLI *, Sequence *sequence, const char *);
    } table[] = {
        {"mid", CLIExportSMF},
        {"midi", CLIExportSMF},
        {"smf", CLIExportSMF},
        {"wav", CLIExportWAV},
        {"wave", CLIExportWAV},
        {"mp3", CLIExportMP3},
    };

    CLIError error = CLIErrorExportWithUnsupportedFileType;
    const char *ext = NAUtilGetFileExtenssion(output);

    for (int i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
        if (0 == strcmp(table[i].ext, ext)) {
            error = table[i].function(self, result.sequence, output);
        }
    }

    SequenceRelease(result.sequence);
    return error;
}

static CLIError CLIExportSMF(CLI *self, Sequence *sequence, const char *output)
{
    Exporter *exporter = ExporterCreate(sequence);
    bool success = ExporterWriteToSMF(exporter, output);
    ExporterDestroy(exporter);
    return success ? CLIErrorNoError : CLIErrorExportWithCannotWriteToOutputFile;
}

static CLIError CLIExportWAV(CLI *self, Sequence *sequence, const char *output)
{
    if (!self->soundSource) {
        return CLIErrorExportWithNoSoundSource;
    }

    if (!MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(self->manager, self->soundSource)) {
        return CLIErrorExportWithSoundSourceLoadFailed;
    }

    Exporter *exporter = ExporterCreate(sequence);
    bool success = ExporterWriteToWave(exporter, output);
    ExporterDestroy(exporter);
    return success ? CLIErrorNoError : CLIErrorExportWithCannotWriteToOutputFile;
}

static CLIError CLIExportMP3(CLI *self, Sequence *sequence, const char *output)
{
    printf("TODO: MP3 output is not implemented yet. (-- )v\n");
    return CLIErrorNoError;
}


static void CLINAMidiOnParseFinish(void *receiver, Sequence *sequence)
{
    CLI *self = receiver;
}

static void CLINAMidiOnParseError(void *receiver, ParseError *error)
{
    printf("%s: %d:%s %s - %d:%d\n", __FUNCTION__, error->kind, ParseErrorKind2String(error->kind), error->location.filepath, error->location.line, error->location.column);
}

static NAMidiObserverCallbacks CLINAMidiObserverCallbacks = {
    CLINAMidiOnParseFinish,
    CLINAMidiOnParseError
};

static void CLIMidiSourceManagerOnLoadMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
    if (!description->available) {
        printf("[%s] is not available. error=%s\n", description->filepath, MidiSourceDescriptionError2String(description->error));
    }
    else {
        printf("[%s] is loaded\n", description->name);
    }
}

static void CLIMidiSourceManagerOnLoadAvailableMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
}

static void CLIMidiSourceManagerOnUnloadMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
    if (!description->available) {
        printf("[%s] is unloaded\n", description->filepath);
    }
    else {
        printf("[%s] is unloaded\n", description->name);
    }
}

static void CLIMidiSourceManagerOnUnloadAvailableMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
}

static MidiSourceManagerObserverCallbacks CLIMidiSourceManagerObserverCallbacks = {
    CLIMidiSourceManagerOnLoadMidiSourceDescription,
    CLIMidiSourceManagerOnLoadAvailableMidiSourceDescription,
    CLIMidiSourceManagerOnUnloadMidiSourceDescription,
    CLIMidiSourceManagerOnUnloadAvailableMidiSourceDescription,
};
