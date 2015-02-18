#pragma once

#include <NAType.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct _NAMidi NAMidi;
NAExportClass(NAMidi);

extern NAMidi *NAMidiCreate();
extern void NAMidiAddContextView(NAMidi *self, void *contextView);
extern void NAMidiSetFile(NAMidi *self, CFStringRef filepath);
extern void NAMidiParse(NAMidi *self);
extern void NAMidiPlay(NAMidi *self);
extern void NAMidiStop(NAMidi *self);
extern void NAMidiPlayPause(NAMidi *self);
extern void NAMidiRewind(NAMidi *self);
extern void NAMidiForward(NAMidi *self);
extern void NAMidiBackward(NAMidi *self);
