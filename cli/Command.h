#pragma once

#include "NAMidi.h"

typedef struct _Command Command;

extern Command *CommandParse(const char *line);
extern void CommandExecute(Command *self, NAMidi *namidi);
