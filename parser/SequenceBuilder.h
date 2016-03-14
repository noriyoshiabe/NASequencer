#pragma once

typedef struct _SequenceBuilder {
    void (*destroy)(void *self);

    void (*setResolution)(void *self, int resolution);
    void (*appendTitle)(void *self, int tick, const char *title);
    void (*appendCopyright)(void *self, int tick, const char *text);
    void (*appendTempo)(void *self, int tick, float tempo);
    void (*appendTimeSign)(void *self, int tick, int numerator, int denominator);
    void (*appendKey)(void *self, int tick, int sf, int mi);
    void (*appendNote)(void *self, int tick, int channel, int noteNo, int gatetime, int velocity);
    void (*appendMarker)(void *self, int tick, const char *marker);
    void (*appendBank)(void *self, int tick, int channel, int bankNo);
    void (*appendProgram)(void *self, int tick, int channel, int programNo);
    void (*appendSynth)(void *self, int tick, int channel, const char *identifier);
    void (*appendVolume)(void *self, int tick, int channel, int value);
    void (*appendPan)(void *self, int tick, int channel, int value);
    void (*appendChorus)(void *self, int tick, int channel, int value);
    void (*appendReverb)(void *self, int tick, int channel, int value);
    void (*appendExpression)(void *self, int tick, int channel, int value);
    void (*appendDetune)(void *self, int tick, int channel, int value);
    void (*setLength)(void *self, int length);

    void *(*build)(void *self);
} SequenceBuilder;
