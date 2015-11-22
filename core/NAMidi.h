#pragma once

#include "Sequence.h"
#include "BuildInformation.h"
#include "Parser.h"
#include "Mixer.h"
#include "Player.h"

typedef struct _NAMidi NAMidi;

typedef struct _NAMidiObserverCallbacks {
    void (*onParseFinish)(void *receiver, Sequence *sequence, BuildInformation *info);
} NAMidiObserverCallbacks;

extern NAMidi *NAMidiCreate();
extern void NAMidiDestroy(NAMidi *self);
extern void NAMidiAddObserver(NAMidi *self, void *receiver, NAMidiObserverCallbacks *callbacks);
extern void NAMidiRemoveObserver(NAMidi *self, void *receiver);
extern void NAMidiSetWatchEnable(NAMidi *self, bool watchEnable);
extern void NAMidiParse(NAMidi *self, const char *filepath);

extern Player *NAMidiGetPlayer(NAMidi *self);
extern Mixer *NAMidiGetMixer(NAMidi *self);
extern Sequence *NAMidiGetSequence(NAMidi *self);
