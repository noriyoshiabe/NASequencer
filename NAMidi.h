#pragma once

#include <NAType.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct _NAMidi NAMidi;
NAExportClass(NAMidi);

extern void NAMidiAddContextView(NAMidi *self, void *contextView);
extern void NAMidiSetFile(NAMidi *self, CFStringRef filepath);
extern void NAMidiStart(NAMidi *self);
