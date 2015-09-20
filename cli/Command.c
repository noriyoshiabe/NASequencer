#include "Command.h"
#include "NAArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

struct _Command {
    void (*execute)(Command *self, NAMidi *namidi);
    NAArray *argv;
};

typedef struct _CommandTable {
    const char *cmd;
    void (*execute)(Command *, NAMidi *);
} CommandTable;

static CommandTable commandTable[];

static void CommandDestroy(Command *self);

static void EmptyCommandExecute(Command *self, NAMidi *namidi)
{
}

static void UnknownCommandExecute(Command *self, NAMidi *namidi)
{
    printf("Unknown command: %s\n", NAArrayGetValueAt(self->argv, 0));
}

static void PlayCommandExecute(Command *self, NAMidi *namidi)
{
    Player *player = NAMidiGetPlayer(namidi);
    PlayerPlay(player);
}

static void StopCommandExecute(Command *self, NAMidi *namidi)
{
    Player *player = NAMidiGetPlayer(namidi);
    PlayerStop(player);
}

static void RewindCommandExecute(Command *self, NAMidi *namidi)
{
    Player *player = NAMidiGetPlayer(namidi);
    PlayerRewind(player);
}

static void ForwardCommandExecute(Command *self, NAMidi *namidi)
{
    Player *player = NAMidiGetPlayer(namidi);
    PlayerForward(player);
}

static void BackwardCommandExecute(Command *self, NAMidi *namidi)
{
    Player *player = NAMidiGetPlayer(namidi);
    PlayerBackWard(player);
}

static Command *CommandCreate(void (*execute)(Command *, NAMidi *), NAArray *argv)
{
    Command *self = calloc(1, sizeof(Command));
    self->execute = execute;
    self->argv = argv;
    return self;
}

Command *CommandParse(const char *line)
{
    int length = strlen(line);
    char *str = alloca(length + 1);
    strcpy(str, line);

    NAArray *argv = NAArrayCreate(4, NADescriptionCString);

    char *saveptr, *token, *s = str;
    while ((token = strtok_r(s, " ", &saveptr))) {
        NAArrayAppend(argv, strdup(token));
        s = NULL;
    }

    void (*execute)(Command *, NAMidi *);

    if (0 < NAArrayCount(argv)) {
        for (int i = 0; NULL != commandTable[i].cmd; ++i) {
            if (0 == strcmp(commandTable[i].cmd, NAArrayGetValueAt(argv, 0))) {
                execute = commandTable[i].execute;
                break;
            }
        }

        if (!execute) {
            execute = UnknownCommandExecute;
        }
    }
    else {
        execute = EmptyCommandExecute;
    }

    return CommandCreate(execute, argv);
}

void CommandExecute(Command *self, NAMidi *namidi)
{
    self->execute(self, namidi);
    CommandDestroy(self);
}

static void CommandDestroy(Command *self)
{
    NAArrayTraverse(self->argv, free);
    NAArrayDestroy(self->argv);
    free(self);
}

char *CommandCompletionEntry(const char *text, int state)
{
    static int index, length;

    if (0 == state) {
        index = 0;
        length = strlen(text);
    }

    const char *cmd;
    while ((cmd = commandTable[index].cmd)) {
        ++index;

        if (0 == strncmp(text, cmd, length)) {
            return strdup(cmd);
        }
    }

    return NULL;
}

static CommandTable commandTable[] = {
    {"play", PlayCommandExecute},
    {"stop", StopCommandExecute},
    {"rewind", RewindCommandExecute},
    {"forward", ForwardCommandExecute},
    {"backward", BackwardCommandExecute},

    {NULL, NULL}
};
