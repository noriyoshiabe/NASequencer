#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "TimeTable.h"
#include "ParseError.h"

typedef enum {
    NAMidiParserEventTypeNote,
    NAMidiParserEventTypeTime,
    NAMidiParserEventTypeTempo,
    NAMidiParserEventTypeSound,
    NAMidiParserEventTypeMarker,
} NAMidiParserEventType;

typedef struct _NAMidiParser NAMidiParser;

typedef struct {
    void (*onParseEvent)(void *receiver, NAMidiParserEventType type, ...);
    void (*onFinish)(void *receiver, TimeTable *timeTable);
    void (*onError)(void *receiver, const char *filepath, int line, int column, ParseError error, const void *info);
} NAMidiParserCallbacks;

extern NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks, void *receiver);
extern void NAMidiParserDestroy(NAMidiParser *self);
extern bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath);
