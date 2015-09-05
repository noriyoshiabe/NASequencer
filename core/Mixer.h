#pragma once

#include "MidiEvent.h"

#include <stdint.h>

typedef struct _Mixer Mixer;

typedef struct _MixerCallbacks {
} MixerCallbacks;

extern Mixer *MixerCreate();
extern void MixerDestroy(Mixer *self);
extern void MixerAddObserver(Mixer *self, void *receiver, MixerCallbacks *callbacks);
extern void MixerRemoveObserver(Mixer *self, void *receiver);

extern void MixerSendNoteOn(Mixer *self, NoteEvent *event);
extern void MixerSendNoteOff(Mixer *self, NoteEvent *event);
extern void MixerSendAllNoteOff(Mixer *self);
extern void MixerSendVoice(Mixer *self, VoiceEvent *event);
extern void MixerSendVolume(Mixer *self, VolumeEvent *event);
extern void MixerSendPan(Mixer *self, PanEvent *event);
extern void MixerSendChorus(Mixer *self, ChorusEvent *event);
extern void MixerSendReverb(Mixer *self, ReverbEvent *event);
