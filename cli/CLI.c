#include "CLI.h"
#include "Command.h"
#include "Exporter.h"
#include "Parser.h"
#include "NAMidiParser.h"
#include "ABCParser.h"

#include "NAUtil.h"
#include "NACString.h"

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
    char *filepath;
    NAMidi *namidi;
    MidiSourceManager *manager;
    PianoRollView *pianoRollView;
    EventListView *eventListView;
    void *activeView;
    sigjmp_buf sigjmpBuf;
    jmp_buf jmpBuf;
    bool prompt;
};

static NAMidiObserverCallbacks CLINAMidiObserverCallbacks;
static PlayerObserverCallbacks CLIPlayerObserverCallbacks;
static MidiSourceManagerObserverCallbacks CLIMidiSourceManagerObserverCallbacks;
static ExporterObserverCallbacks CLIExporterObserverCallbacks;

static char **CLICompletion(const char *text, int start, int end);
static char *CLIFormatParseError(CLI *self, const ParseError *error);

CLI *CLICreate(const char *filepath, const char **soundSources)
{
    CLI *self = calloc(1, sizeof(CLI));
    self->filepath = filepath ? strdup(filepath) : NULL;
    self->namidi = NAMidiCreate();
    self->manager = MidiSourceManagerSharedInstance();
    self->pianoRollView = PianoRollViewCreate(self->namidi);
    self->eventListView = EventListViewCreate(self->namidi);
    self->activeView = self->pianoRollView;
    NAMidiAddObserver(self->namidi, self, &CLINAMidiObserverCallbacks);
    PlayerAddObserver(NAMidiGetPlayer(self->namidi), self, &CLIPlayerObserverCallbacks);
    MidiSourceManagerAddObserver(self->manager, self, &CLIMidiSourceManagerObserverCallbacks);

    for (const char **source = soundSources; NULL != *source; ++source) {
        MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(self->manager, *source);
    }

    return self;
}

void CLIDestroy(CLI *self)
{
    PianoRollViewDestroy(self->pianoRollView);
    EventListViewDestroy(self->eventListView);
    MidiSourceManagerRemoveObserver(self->manager, self);
    NAMidiRemoveObserver(self->namidi, self);
    NAMidiDestroy(self->namidi);

    if (self->filepath) {
        free(self->filepath);
    }

    free(self);
}

bool CLIRunShell(CLI *self)
{
    char historyFile[PATH_MAX];
    char *line = NULL;
    int historyCount = 0;

    if (self->filepath) {
        NAMidiSetWatchEnable(self->namidi, true);
        NAMidiParse(self->namidi, self->filepath);
    }

    sprintf(historyFile, "%s/.namidi_history", getenv("HOME"));
    read_history(historyFile);

    while (sigsetjmp(self->sigjmpBuf, 1));

    if (setjmp(self->jmpBuf)) {
        goto EXIT;
    }

    rl_attempted_completion_function = CLICompletion;

    self->prompt = true;
    while ((line = readline(PROMPT))) {
        self->prompt = false;

        Command *cmd = CommandParse(line);
        CommandExecute(cmd, self);

        if ('\0' != line[0]) {
            add_history(line);

            if (MAX_HISTORY < ++historyCount) {
                free(remove_history(0));
            }
        }

        free(line);

        self->prompt = true;
    }

EXIT:
    printf("\n");

    write_history(historyFile);
    clear_history();

    return true;
}

static char **CLICompletion(const char *text, int start, int end)
{
    if (0 == start) {
        rl_completion_append_character = ' ';
        return rl_completion_matches(text, CommandCompletionEntry);
    }
    else {
        rl_completion_append_character = '\0';

        char **matches = rl_completion_matches(text, rl_filename_completion_function);
        for (int i = 0; matches && matches[i]; ++i) {
            if (NAUtilIsDirectory(matches[i])) {
                char *str = malloc(strlen(matches[i]) + 2);
                strcpy(str, matches[i]);
                if (str[strlen(str) - 1] != '/') {
                    strcat(str, "/");
                }
                free(matches[i]);
                matches[i] = str;
            }

            int count = 0;
            for (int j = 0; matches[i][j]; ++j) {
                if (matches[i][j] == ' ') {
                    ++count;
                }
            }

            if (0 < count) {
                char *str = malloc(strlen(matches[i]) + count + 1);
                str[0] = '\0';

                char *saveptr, *token, *s = matches[i];
                while ((token = strtok_r(s, " ", &saveptr))) {
                    strcat(str, token);
                    if (0 < count--) {
                        strcat(str, "\\ ");
                    }
                    s = NULL;
                }

                free(matches[i]);
                matches[i] = str;
            }
        }

        return matches;
    }
}

void CLISigInt(CLI *self)
{
    printf("\n");
    siglongjmp(self->sigjmpBuf, 1);
}

void CLIExit(CLI *self)
{
    longjmp(self->jmpBuf, 1);
}

bool CLIExport(CLI *self, const char *output)
{
    Exporter *exporter = ExporterCreate(&CLIExporterObserverCallbacks, self);
    ExporterError error = ExporterExport(exporter, self->filepath, output);
    ExporterDestroy(exporter);
    
    switch (error) {
    case ExporterErrorUnsupportedFileType:
        fprintf(stderr, "unsupported output file type.\n");
        break;
    case ExporterErrorNoSoundSource:
        fprintf(stderr, "no synthesizer is loaded.\n");
        break;
    case ExporterErrorParseFailed:
        fprintf(stderr, "parse failed.\n");
        break;
    case ExporterErrorCouldNotWriteFile:
        fprintf(stderr, "cannot write to output file.\n");
        break;
    default:
        break;
    }

    return ExporterErrorNoError == error;
}

NAMidi *CLIGetNAMidi(CLI *self)
{
    return self->namidi;
}

PianoRollView *CLIGetPianoRollView(CLI *self)
{
    return self->pianoRollView;
}

EventListView *CLIGetEventListView(CLI *self)
{
    return self->eventListView;
}

void CLISetActiveView(CLI *self, void *view)
{
    self->activeView = view;
}

void CLISetFilepath(CLI *self, const char *filepath)
{
    if (self->filepath) {
        free(self->filepath);
    }
    self->filepath = strdup(filepath);
}


static void CLINAMidiOnBeforeParse(void *receiver, bool fileChanged)
{
    if (fileChanged) {
        fprintf(stdout, "\n");
    }
}

static void CLINAMidiOnReadFile(void *receiver, const char *filepath)
{
    fprintf(stdout, "reading %s.\n", filepath);
}

static void CLINAMidiOnParseError(void *receiver, const ParseError *error)
{
}

static void CLINAMidiOnParseFinish(void *receiver, bool success, Sequence *sequence, ParseInfo *info)
{
    CLI *self = receiver;

    PianoRollViewSetSequence(self->pianoRollView, sequence);
    EventListViewSetSequence(self->eventListView, sequence);

    if (self->activeView == self->pianoRollView) {
        PianoRollViewRender(self->pianoRollView);
    }
    else if (self->activeView == self->eventListView) {
        EventListViewRender(self->eventListView);
    }

    if (success) {
        fprintf(stdout, "parse finished.\n");
    }
    else {
        fprintf(stdout, "parse failed.\n");
    }

    NAIterator *iterator = NAArrayGetIterator(info->errors);
    while (iterator->hasNext(iterator)) {
        ParseError *error = iterator->next(iterator);
        char *formatted = CLIFormatParseError(receiver, error);
        fputs(formatted, stderr);
        free(formatted);
    }

    if (self->prompt) {
        fprintf(stdout, PROMPT);
        fflush(stdout);
    }
}

static NAMidiObserverCallbacks CLINAMidiObserverCallbacks = {
    CLINAMidiOnBeforeParse,
    CLINAMidiOnReadFile,
    CLINAMidiOnParseError,
    CLINAMidiOnParseFinish,
};

static void CLIPlayerOnNotifyClock(void *receiver, int tick, int64_t usec, Location location)
{
}

static void CLIPlayerOnNotifyEvent(void *receiver, PlayerEvent event)
{
#if 0
    printf("%s\n", PlayerEvent2String(event));
#endif
}

static void CLIPlayerOnSendNoteOn(void *receiver, NoteEvent *event)
{
}

static void CLIPlayerOnSendNoteOff(void *receiver, NoteEvent *event)
{
}

static PlayerObserverCallbacks CLIPlayerObserverCallbacks = {
    CLIPlayerOnNotifyClock,
    CLIPlayerOnNotifyEvent,
    CLIPlayerOnSendNoteOn,
    CLIPlayerOnSendNoteOff
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

static void CLIExporterOnParseError(void *receiver, const ParseError *error)
{
    char *formatted = CLIFormatParseError(receiver, error);
    fputs(formatted, stderr);
    free(formatted);
}

static void CLIExporterOnParseFinish(void *_self, ParseInfo *info)
{
}

static void CLIExporterOnProgress(void *_self, int progress)
{
    for (int i = 0; i < progress; ++i) {
        fprintf(stderr, ".");
    }
    fprintf(stderr, "%d%%%s", progress, 100 == progress ? "\n" : "\r");
}

static ExporterObserverCallbacks CLIExporterObserverCallbacks = {
    CLIExporterOnParseError,
    CLIExporterOnParseFinish,
    CLIExporterOnProgress,
};

static char *CLIFormatParseError(CLI *self, const ParseError *error)
{
    char location[256];
    char head[32];

    snprintf(location, 256, "%s:%d:%d", error->location.filepath, error->location.line, error->location.column);
    snprintf(head, 32, "[ERROR:%d]", error->code);

    switch (error->code) {
    case GeneralParseErrorUnsupportedFileType:
    case GeneralParseErrorFileNotFound:
    case GeneralParseErrorSyntaxError:
        // TODO
        break;
    case NAMidiParseErrorInvalidResolution:
    case NAMidiParseErrorInvalidTempo:
    case NAMidiParseErrorInvalidTimeSign:
    case NAMidiParseErrorInvalidChannel:
    case NAMidiParseErrorInvalidVoice:
    case NAMidiParseErrorInvalidVolume:
    case NAMidiParseErrorInvalidPan:
    case NAMidiParseErrorInvalidChorus:
    case NAMidiParseErrorInvalidReverb:
    case NAMidiParseErrorInvalidTranspose:
    case NAMidiParseErrorInvalidKeySign:
    case NAMidiParseErrorInvalidStep:
    case NAMidiParseErrorInvalidNoteNumber:
    case NAMidiParseErrorInvalidOctave:
    case NAMidiParseErrorInvalidGatetime:
    case NAMidiParseErrorInvalidVelocity:
    case NAMidiParseErrorUnsupportedFileTypeInclude:
    case NAMidiParseErrorIncludeFileNotFound:
    case NAMidiParseErrorCircularFileInclude:
    case NAMidiParseErrorPatternMissing:
    case NAMidiParseErrorDuplicatePatternIdentifier:
    case NAMidiParseErrorCircularPatternReference:
        // TODO
        break;
    case ABCParseErrorUnrecognisedVersion:
    case ABCParseErrorUnexpectedVersionExpression:
    case ABCParseErrorInvalidKeyMode:
    case ABCParseErrorInvalidKeySign:
    case ABCParseErrorInvalidNoteNumber:
    case ABCParseErrorIllegalOctaveDown:
    case ABCParseErrorIllegalOctaveUp:
        // TODO
        break;
    default:
        break;
    }

    char params[256] = {0};
    for (int i = 0; i < 4 && error->infos[i]; ++i) {
        if (0 < i) {
            strcat(params, ",");
        }
        strcat(params, error->infos[i]);
    }

    return NACStringFormat("%s %s %s at %s\n", head, ParseError2String(error), params, location);
}
