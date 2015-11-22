#include "CLI.h"
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

static char **CLICompletion(const char *text, int start, int end);

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

#include "Parser.h"
#include "SequenceBuilderImpl.h"
#include "BuildInformation.h"

static bool CLIExportSMF(CLI *self, Sequence *sequence, const char *output);
static bool CLIExportWAV(CLI *self, Sequence *sequence, const char *output);
static bool CLIExportAAC(CLI *self, Sequence *sequence, const char *output);

bool CLIExport(CLI *self, const char *output)
{
    if (!self->filepath) {
        fprintf(stderr, "no input source file\n");
        return false;
    }

    SequenceBuilder *builder = SequenceBuilderCreate();
    Parser *parser = ParserCreate(builder);
    Sequence *sequence = NULL;
    BuildInformation *info = NULL;

    bool success = ParserParseFile(parser, self->filepath, (void **)&sequence, (void **)&info);

    ParserDestroy(parser);
    builder->destroy(builder);
    
    if (!success) {
        NAIterator *iterator = NAArrayGetIterator(info->errors);
        while (iterator->hasNext(iterator)) {
            ParseError *error = iterator->next(iterator);
            fprintf(stderr, "parse error. %s - %s:%d:%d\n", ParseError2String(error), error->location.filepath, error->location.line, error->location.column);
        }
        goto ERROR;
    }
            
    const struct {
        const char *ext;
        bool (*function)(CLI *, Sequence *sequence, const char *);
    } table[] = {
        {"mid", CLIExportSMF},
        {"midi", CLIExportSMF},
        {"smf", CLIExportSMF},
        {"wav", CLIExportWAV},
        {"wave", CLIExportWAV},
        {"m4a", CLIExportAAC},
    };

    bool (*function)(CLI *, Sequence *sequence, const char *) = NULL;
    const char *ext = NAUtilGetFileExtenssion(output);

    for (int i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
        if (0 == strcmp(table[i].ext, ext)) {
            function = table[i].function;
            break;
        }
    }

    if (!function) {
        fprintf(stderr, "unsupported output file type.\n");
        goto ERROR;
    }

    success = function(self, sequence, output);

ERROR:
    if (sequence) {
        SequenceRelease(sequence);
    }

    if (info) {
        BuildInformationRelease(info);
    }

    return success;
}

static void CLIExporterProgressCallback(void *_self, int progress)
{
    for (int i = 0; i < progress; ++i) {
        fprintf(stderr, ".");
    }
    fprintf(stderr, "%d%%%s", progress, 100 == progress ? "\n" : "\r");
}

static bool CLIHasSoundSource(CLI *self)
{
    NAArray *descriptions = MidiSourceManagerGetAvailableDescriptions(self->manager);
    return 0 < NAArrayCount(descriptions);
}

static bool CLIExportSMF(CLI *self, Sequence *sequence, const char *output)
{
    Exporter *exporter = ExporterCreate(sequence);
    bool success = ExporterWriteToSMF(exporter, output);
    ExporterDestroy(exporter);
    return success;
}

static bool CLIExportWAV(CLI *self, Sequence *sequence, const char *output)
{
    if (!CLIHasSoundSource(self)) {
        fprintf(stderr, "no synthesizer is loaded.\n");
        return false;
    }

    Exporter *exporter = ExporterCreate(sequence);
    ExporterSetProgressCallback(exporter, CLIExporterProgressCallback, self);
    bool success = ExporterWriteToWave(exporter, output);
    if (!success) {
        fprintf(stderr, "cannot write to output file.\n");
    }
    ExporterDestroy(exporter);
    return success;
}

static bool CLIExportAAC(CLI *self, Sequence *sequence, const char *output)
{
    if (!CLIHasSoundSource(self)) {
        fprintf(stderr, "no synthesizer is loaded.\n");
        return false;
    }

    Exporter *exporter = ExporterCreate(sequence);
    ExporterSetProgressCallback(exporter, CLIExporterProgressCallback, self);
    bool success = ExporterWriteToAAC(exporter, output);
    if (!success) {
        fprintf(stderr, "cannot write to output file.\n");
    }
    ExporterDestroy(exporter);
    return success;
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


static void CLINAMidiOnParseFinish(void *receiver, Sequence *sequence, BuildInformation *info)
{
    CLI *self = receiver;

    if (self->prompt) {
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "parse finished.\n");

    PianoRollViewSetSequence(self->pianoRollView, sequence);
    EventListViewSetSequence(self->eventListView, sequence);

    if (self->activeView == self->pianoRollView) {
        PianoRollViewRender(self->pianoRollView);
    }
    else if (self->activeView == self->eventListView) {
        EventListViewRender(self->eventListView);
    }

    NAIterator *iterator = NAArrayGetIterator(info->errors);
    while (iterator->hasNext(iterator)) {
        ParseError *error = iterator->next(iterator);
        fprintf(stderr, "parse error. %s - %s:%d:%d\n", ParseError2String(error), error->location.filepath, error->location.line, error->location.column);
    }

    if (self->prompt) {
        fprintf(stdout, PROMPT);
        fflush(stdout);
    }
}

static NAMidiObserverCallbacks CLINAMidiObserverCallbacks = {
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
