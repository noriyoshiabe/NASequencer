#pragma once

#include <CoreMIDI/CoreMIDI.h>
#include <NAType.h>

typedef struct _MidiClient MidiClient;
NAExportClass(MidiClient);

extern bool MidiClientOpen(MidiClient *self);
extern void MidiClientSend(MidiClient *self, uint8_t *bytes, size_t length);
extern void MidiClientClose(MidiClient *self);
extern bool MidiClientIsAvailable(MidiClient *self);
