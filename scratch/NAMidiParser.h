#pragma once

#include <stdint.h>

typedef struct _NAMidiParser NAMidiParser;

typedef struct {
    void (*onParseResolution)(uint16_t resolution);
    void (*onParseTitle)(const char *text);
    void (*onParseNote)(uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime);
    void (*onParseTime)(uint32_t tick, uint8_t numerator, uint8_t denominator);
    void (*onParseTempo)(uint32_t tick, float tempo);
    void (*onParseMarker)(uint32_t tick, const char *text);
    void (*onParseSoundSelect)(uint32_t tick, uint8_t channel, uint8_t msb, uint8_t lsb, uint8_t programNo);

    void (*onFinish)(uint32_t length);
    void (*onError)(const char *filepath, int line, int column, NAMidiParserError error, ...);
} NAMidiParserCallbacks;

extern NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks);
extern void NAMidiParserDestroy(NAMidiParser *self);
extern void NAMidiParserExecuteParse(NAMidiParser *self, const char *filename);
