#pragma once

#include "NAMidiParser.h"

typedef struct _NAMidiInterpreter NAMidiInterpreter;

typedef enum {
    NAMidiInterpreterErrorNoError;
} NAMidiInterpreterError;

extern NAMidiInterpreter *NAMidiInterpreterCreate();
extern void NAMidiInterpreterDestroy(NAMidiInterpreter *self);
extern bool NAMidiInterpreterInterpretParseEvent(NAMidiInterpreter *self, NAMidiInterpreterError *error);
