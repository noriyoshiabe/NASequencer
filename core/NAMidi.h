#pragma once

#include <NAType.h>
#include <CoreFoundation/CoreFoundation.h>
#include "ParseContext.h"

typedef struct _NAMidiObserverVtbl {
    void (*onParseFinished)(void *self, ParseContext *context);
    void (*onPlayingStateChanged)(void *self, void *unimplemented);
} NAMidiObserverVtbl;

NAExportClass(NAMidiObserver);

typedef struct _NAMidi NAMidi;
NAExportClass(NAMidi);

extern NAMidi *NAMidiCreate();
extern void NAMidiAddObserver(NAMidi *self, void *observer);
extern void NAMidiSetFile(NAMidi *self, CFStringRef filepath);
extern void NAMidiParse(NAMidi *self);
extern void NAMidiPlay(NAMidi *self);
extern void NAMidiStop(NAMidi *self);
extern void NAMidiPlayPause(NAMidi *self);
extern void NAMidiRewind(NAMidi *self);
extern void NAMidiForward(NAMidi *self);
extern void NAMidiBackward(NAMidi *self);
