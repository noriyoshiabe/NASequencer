#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "TimeTable.h"
#include "ParseError.h"

typedef struct _NAMidiParser NAMidiParser;

typedef struct {
    void (*onParseResolution)(void *receiver, uint32_t resolution);
    void (*onParseNote)(void *receiver, uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime);
    void (*onParseTime)(void *receiver, uint32_t tick, uint8_t numerator, uint8_t denominator);
    void (*onParseTempo)(void *receiver, uint32_t tick, float tempo);
    void (*onParseMarker)(void *receiver, uint32_t tick, const char *text);

    void (*onFinish)(void *receiver, TimeTable *timeTable);
    void (*onError)(void *receiver, const char *filepath, int line, int column, ParseError error, const void *info);
} NAMidiParserCallbacks;

extern NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks, void *receiver);
extern void NAMidiParserDestroy(NAMidiParser *self);
extern bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath);
