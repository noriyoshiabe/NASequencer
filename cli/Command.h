#pragma once

#include "CLI.h"

typedef struct _Command Command;

extern Command *CommandParse(const char *line);
extern void CommandExecute(Command *self, CLI *cli);
extern char *CommandCompletionEntry(const char *text, int state);
extern void CommandShowHelp();
