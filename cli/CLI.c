#include "CLI.h"
#include "NAMidi.h"
#include "Command.h"

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
    sigjmp_buf jmpBuf;
};

static void CLINAMidiOnParseFinish(void *receiver, Sequence *sequence)
{
    printf("called %s\n", __FUNCTION__);

    CLI *self = receiver;

    // TODO remove
    PlayerSetSequence(NAMidiGetPlayer(self->namidi), sequence);
    PlayerPlay(NAMidiGetPlayer(self->namidi));
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


CLI *CLICreate(const char *filepath, const char *soundSource)
{
    CLI *self = calloc(1, sizeof(CLI));
    self->filepath = filepath;
    self->soundSource = soundSource;
    self->namidi = NAMidiCreate();
    NAMidiAddObserver(self->namidi, self, &CLINAMidiObserverCallbacks);
    MidiSourceManagerAddObserver(MidiSourceManagerSharedInstance(), self, &CLIMidiSourceManagerObserverCallbacks);
    return self;
}

void CLIRun(CLI *self)
{
    char historyFile[PATH_MAX];
    char *line = NULL;
    int historyCount = 0;

    if (self->soundSource) {
        MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(MidiSourceManagerSharedInstance(), self->soundSource);
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
}

void CLISigInt(CLI *self)
{
    printf("\n");
    siglongjmp(self->jmpBuf, 1);
}

extern void CLIDestroy(CLI *self)
{
    MidiSourceManagerRemoveObserver(MidiSourceManagerSharedInstance(), self);
    NAMidiRemoveObserver(self->namidi, self);
    NAMidiDestroy(self->namidi);
    free(self);
}
