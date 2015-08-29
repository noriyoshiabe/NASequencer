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
    NAMidi *namidi;
    sigjmp_buf jmpBuf;
};

extern void CLINAMidiOnParseFinish(void *receiver, Sequence *sequence)
{
    printf("called %s\n", __FUNCTION__);
}

extern void CLINAMidiOnParseError(void *receiver, ParseError *error)
{
    printf("%s: %d:%s\n", __FUNCTION__, error->kind, ParseErrorKind2String(error->kind));
}

static NAMidiObserverCallbacks CLINAMidiObserverCallbacks = {
    CLINAMidiOnParseFinish,
    CLINAMidiOnParseError
};

CLI *CLICreate(const char *filepath)
{
    CLI *self = calloc(1, sizeof(CLI));
    self->filepath = filepath;
    self->namidi = NAMidiCreate();
    NAMidiAddObserver(self->namidi, self, &CLINAMidiObserverCallbacks);
    return self;
}

void CLIRun(CLI *self)
{
    char historyFile[PATH_MAX];
    char *line = NULL;
    int historyCount = 0;

    if (self->filepath) {
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
    NAMidiRemoveObserver(self->namidi, self);
    NAMidiDestroy(self->namidi);
    free(self);
}
