#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ParseErrorFileNotFound,
    ParseErrorInitError,
    ParseErrorSyntaxError,
} ParseError;

typedef struct _NAMidiParser NAMidiParser;

typedef struct {
    void (*onParseNote)(void *context, uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime);
    void (*onParseTime)(void *context, uint32_t tick, uint8_t numerator, uint8_t denominator);
    void (*onParseTempo)(void *context, uint32_t tick, float tempo);
    void (*onParseMarker)(void *context, uint32_t tick, const char *text);

    void (*onFinish)(void *context, uint32_t length);
    void (*onError)(void *context, const char *filepath, int line, int column, ParseError error, ...);
} NAMidiParserCallbacks;

extern NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks, void *context);
extern void NAMidiParserDestroy(NAMidiParser *self);
extern bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath);
