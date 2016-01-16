#pragma once

#include "MidiEvent.h"
#include "NAArray.h"
#include "MidiSource.h"
#include "MidiSourceManager.h"
#include "AudioOut.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct _Mixer Mixer;
typedef struct _MixerChannel MixerChannel;

typedef struct _MixerObserverCallbacks {
    void (*onChannelStatusChange)(void *receiver, MixerChannel *channel);
    void (*onAvailableMidiSourceChange)(void *receiver, NAArray *descriptions);
    void (*onLevelUpdate)(void *receiver);
} MixerObserverCallbacks;

extern Mixer *MixerCreate(AudioOut *audioOut);
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
extern void MixerSendExpression(Mixer *self, ExpressionEvent *event);
extern void MixerSendDetune(Mixer *self, DetuneEvent *event);
extern void MixerSendSynth(Mixer *self, SynthEvent *event);

extern NAArray *MixerGetChannels(Mixer *self);

extern int MixerChannelGetNumber(MixerChannel *self);
extern MidiSourceDescription *MixerChannelGetMidiSourceDescription(MixerChannel *self);
extern int MixerChannelGetPresetCount(MixerChannel *self);
extern PresetInfo **MixerChannelGetPresetInfos(MixerChannel *self);
extern PresetInfo *MixerChannelGetPresetInfo(MixerChannel *self);
extern Level MixerChannelGetLevel(MixerChannel *self);
extern int MixerChannelGetVolume(MixerChannel *self);
extern int MixerChannelGetPan(MixerChannel *self);
extern int MixerChannelGetChorusSend(MixerChannel *self);
extern int MixerChannelGetReverbSend(MixerChannel *self);
extern int MixerChannelGetExpressionSend(MixerChannel *self);
extern bool MixerChannelGetMute(MixerChannel *self);
extern bool MixerChannelGetSolo(MixerChannel *self);

extern void MixerChannelSetMidiSourceDescription(MixerChannel *self, MidiSourceDescription *description);
extern void MixerChannelSetPresetInfo(MixerChannel *self, PresetInfo *presetInfo);
extern void MixerChannelSetVolume(MixerChannel *self, int value);
extern void MixerChannelSetPan(MixerChannel *self, int value);
extern void MixerChannelSetChorusSend(MixerChannel *self, int value);
extern void MixerChannelSetReverbSend(MixerChannel *self, int value);
extern void MixerChannelSetExpressionSend(MixerChannel *self, int value);
extern void MixerChannelSetMute(MixerChannel *self, bool mute);
extern void MixerChannelSetSolo(MixerChannel *self, bool solo);
