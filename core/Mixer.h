#pragma once

#include "MidiEvent.h"
#include "NAArray.h"
#include "MidiSource.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct _Mixer Mixer;

typedef struct _Track {
    int channel;
    MidiSource *source;
    PresetList preset;
    Level level;
    int volume;
    int pan;
    int chorusSend;
    int reverbSend;
    bool mute;
    bool solo;
} Track;

typedef struct _MixerObserverCallbacks {
    void (*onTrackChange)(void *receiver, int trackNo);
    void (*onAvailableMidisourceChange)(void *receiver, NAArray *midiSources);
    void (*onLevelUpdate)(void *receiver);
} MixerObserverCallbacks;

extern Mixer *MixerCreate();
extern void MixerDestroy(Mixer *self);
extern void MixerAddObserver(Mixer *self, void *receiver, MixerObserverCallbacks *callbacks);
extern void MixerRemoveObserver(Mixer *self, void *receiver);

extern void MixerSendNoteOn(Mixer *self, NoteEvent *event);
extern void MixerSendNoteOff(Mixer *self, NoteEvent *event);
extern void MixerSendAllNoteOff(Mixer *self);
extern void MixerSendVoice(Mixer *self, VoiceEvent *event);
extern void MixerSendVolume(Mixer *self, VolumeEvent *event);
extern void MixerSendPan(Mixer *self, PanEvent *event);
extern void MixerSendChorus(Mixer *self, ChorusEvent *event);
extern void MixerSendReverb(Mixer *self, ReverbEvent *event);

extern Track *MixerGetTracks(Mixer *self);
