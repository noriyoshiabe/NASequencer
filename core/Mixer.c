#include "Mixer.h"
#include "Synthesizer.h"
#include "AudioOut.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct _Mixer {
    NAArray *observers;
    Track tracks[16];
};

typedef struct Observer {
    void *receiver;
    MixerObserverCallbacks *callbacks;
} Observer;

Mixer *MixerCreate()
{
    Mixer *self =  calloc(1, sizeof(Mixer));

    self->observers = NAArrayCreate(4, NULL);

    // TODO move to midisource manager
    char filepath[PATH_MAX];
    sprintf(filepath, "%s/.namidi/GeneralUser GS Live-Audigy v1.44.sf2", getenv("HOME"));
    SoundFontError error;
    SoundFont *soundFont = SoundFontRead(filepath, &error);
    AudioOut *audioOut = AudioOutSharedInstance();
    MidiSource *source = (MidiSource *)SynthesizerCreate(soundFont, AudioOutGetSampleRate(audioOut));

    PresetList presetList[source->getPresetCount(source)];
    source->getPresetList(source, presetList);

    for (int i = 0; i < 16; ++i) {
        int channel = i + 1;
        self->tracks[i].channel = channel;
        self->tracks[i].source = source;
        self->tracks[i].preset = presetList[0];
        self->tracks[i].level = source->getChannelLevel(source, channel);
        self->tracks[i].volume = source->getVolume(source, channel);
        self->tracks[i].pan = source->getPan(source, channel);
        self->tracks[i].chorusSend = source->getChorusSend(source, channel);
        self->tracks[i].reverbSend = source->getReverbSend(source, channel);
    }

    return self;
}

void MixerDestroy(Mixer *self)
{
    NAArrayTraverse(self->observers, free);
    NAArrayDestroy(self->observers);
    free(self);
}

void MixerAddObserver(Mixer *self, void *receiver, MixerObserverCallbacks *callbacks)
{
    Observer *observer = malloc(sizeof(Observer));
    observer->receiver = receiver;
    observer->callbacks = callbacks;
    NAArrayAppend(self->observers, observer);
}

static int MixerObserverFindComparator(const void *receiver, const void *observer)
{
    return receiver - ((Observer *)observer)->receiver;
}

void MixerRemoveObserver(Mixer *self, void *receiver)
{
    int index = NAArrayFindFirstIndex(self->observers, receiver, MixerObserverFindComparator);
    NAArrayApplyAt(self->observers, index, free);
    NAArrayRemoveAt(self->observers, index);
}

void MixerSendNoteOn(Mixer *self, NoteEvent *event)
{
    int channel = event->channel - 1;
    MidiSource *source = self->tracks[channel].source;
    uint8_t bytes[3] = {0x90 | (0x0F & channel), event->noteNo, event->velocity};
    source->send(source, bytes, 3);

}

void MixerSendNoteOff(Mixer *self, NoteEvent *event)
{
    int channel = event->channel - 1;
    MidiSource *source = self->tracks[channel].source;
    uint8_t bytes[3] = {0x80 | (0x0F & channel), event->noteNo, 0};
    source->send(source, bytes, 3);
}

void MixerSendAllNoteOff(Mixer *self)
{
    uint8_t bytes[3] = {0, 0x7B, 0x00};
    for (int i = 0; i < 16; ++i) {
        MidiSource *source = self->tracks[i].source;
        bytes[0] = 0xB0 | i;
        source->send(source, bytes, 3);
    }
}

void MixerSendVoice(Mixer *self, VoiceEvent *event)
{
    int channel = event->channel - 1;
    MidiSource *source = self->tracks[channel].source;
    uint8_t bytes[3];

    bytes[0] = 0xB0 | (0x0F & channel);
    bytes[1] = 0x00;
    bytes[2] = event->msb;
    source->send(source, bytes, 3);

    bytes[1] = 0x20;
    bytes[2] = event->lsb;
    source->send(source, bytes, 3);

    bytes[0] = 0xC0 | (0x0F & channel);
    bytes[1] = event->programNo;
    source->send(source, bytes, 2);
}

void MixerSendVolume(Mixer *self, VolumeEvent *event)
{
    int channel = event->channel - 1;
    MidiSource *source = self->tracks[channel].source;
    source->setVolume(source, channel, event->value);
}

void MixerSendPan(Mixer *self, PanEvent *event)
{
    int channel = event->channel - 1;
    MidiSource *source = self->tracks[channel].source;
    source->setPan(source, channel, event->value);
}

void MixerSendChorus(Mixer *self, ChorusEvent *event)
{
    int channel = event->channel - 1;
    MidiSource *source = self->tracks[channel].source;
    source->setChorusSend(source, channel, event->value);
}

void MixerSendReverb(Mixer *self, ReverbEvent *event)
{
    int channel = event->channel - 1;
    MidiSource *source = self->tracks[channel].source;
    source->setReverbSend(source, channel, event->value);
}

Track *MixerGetTracks(Mixer *self)
{
    return self->tracks;
}
