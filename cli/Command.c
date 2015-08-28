#include "Command.h"

#include <stdio.h>
#include <stdlib.h>

struct _Command {
    void (*execute)(void *self, NAMidi *namidi);
    void (*destroy)(void *self);
};

typedef struct _UnknownCommand {
    Command cmd;
    const char *line;
} UnknownCommand;

static void UnknownCommandExecute(void *self, NAMidi *namidi)
{
    printf("Unknown command: %s\n", ((UnknownCommand *)self)->line);
}

static Command *UnknownCommandCreate(const char *line)
{
    UnknownCommand *self = calloc(1, sizeof(UnknownCommand));
    self->cmd.execute = UnknownCommandExecute;
    self->cmd.destroy = free;
    self->line = line;
    return (Command *)self;
}

Command *CommandParse(const char *line)
{
    return UnknownCommandCreate(line);
}

void CommandExecute(Command *self, NAMidi *namidi)
{
    self->execute(self, namidi);
    self->destroy(self);
}
