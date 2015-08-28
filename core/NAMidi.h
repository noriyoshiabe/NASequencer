#pragma once

#include "Sequence.h"

// TODO move type definition
typedef struct _ParseError ParseError;
typedef struct _Player Player;
typedef struct _Mixer Mixer;

typedef struct _NAMidi NAMidi;

typedef struct _NAMidiObserverCallbacks {
    void (*onParseFinish)(void *receiver, Sequence *sequence);
    void (*onParseError)(void *receiver, ParseError *error);
} NAMidiObserverCallbacks;

extern NAMidi *NAMidiCreate();
extern void NAMidiDestroy(NAMidi *self);
extern void NAMidiAddObserver(NAMidi *self, void *receiver, NAMidiObserverCallbacks *callbacks);
extern void NAMidiRemoveObserver(NAMidi *self, void *receiver);
extern void NAMidiParse(NAMidi *self, const char *filepath);

extern Player *NAMidiGetPlayer(NAMidi *self);
extern Mixer *NAMidiGetMixer(NAMidi *self);
