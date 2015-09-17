#include "Exporter.h"
#include "SMFWriter.h"
#include "AudioOut.h"
#include "Mixer.h"
#include "WaveWriter.h"
#include "NAArray.h"
#include "NASet.h"

#include <stdlib.h>

struct _Exporter {
    Sequence *sequence;
    NASet *noteOffEvents;
    NAArray *eventsToWrite;
};

typedef struct _ExporterAudioBuffer {
    AudioOut audioOut;
    void *receiver;
    AudioCallback callback;
    AudioSample buffer[64];
} ExporterAudioBuffer;

static ExporterAudioBuffer *ExporterAudioBufferCreate();
static void ExporterAudioBufferDestroy(ExporterAudioBuffer *self);


Exporter *ExporterCreate(Sequence *sequence)
{
    Exporter *self = calloc(1, sizeof(Exporter));
    self->sequence = SequenceRetain(sequence);
    return self;
}

void ExporterDestroy(Exporter *self)
{
    if (self->noteOffEvents) {
        NASetTraverse(self->noteOffEvents, free);
        NASetDestroy(self->noteOffEvents);
    }

    if (self->eventsToWrite) {
        NAArrayDestroy(self->eventsToWrite);
    }

    SequenceRelease(self->sequence);
    free(self);
}

static void ExporterBuildEventsToWrite(Exporter *self)
{
    int count = NAArrayCount(self->sequence->events);
    MidiEvent **events = NAArrayGetValues(self->sequence->events);

    self->noteOffEvents = NASetCreate(NAHashAddress, NULL);
    self->eventsToWrite = NAArrayCreate(((count * 2 / 1024) + 1) * 1024, NULL);

    for (int i = 0; i < count; ++i) {
        NAArrayAppend(self->eventsToWrite, events[i]);

        if (MidiEventTypeNote == events[i]->type) {
            NoteEvent *noteOn = (NoteEvent *)events[i];

            NoteEvent *noteOff = MidiEventAlloc(MidiEventTypeNote, noteOn->tick + noteOn->gatetime, sizeof(NoteEvent) - sizeof(MidiEvent));
            noteOff->channel = noteOn->channel;
            noteOff->noteNo = noteOn->noteNo;
            noteOff->velocity = 0;

            NASetAdd(self->noteOffEvents, noteOff);
            NAArrayAppend(self->eventsToWrite, noteOff);
        }
    }

    NAArraySort(self->eventsToWrite, MidiEventComparator);
}

bool ExporterWriteToSMF(Exporter *self, const char *filepath)
{
    SMFWriter *writer = SMFWriterCreate();
    if (!SMFWriterOpenFile(writer, filepath)) {
        SMFWriterDestroy(writer);
        return false;
    }

    ExporterBuildEventsToWrite(self);

    int count = NAArrayCount(self->eventsToWrite);
    MidiEvent **events = NAArrayGetValues(self->eventsToWrite);

    SMFWriterSetResolution(writer, TimeTableResolution(self->sequence->timeTable));

    for (int i = 0; i < count; ++i) {
        switch (events[i]->type) {
        case MidiEventTypeNote:
            {
                NoteEvent *note = (NoteEvent *)events[i];
                if (NASetContains(self->noteOffEvents, note)) {
                    SMFWriterAppendNoteOff(writer, note->tick, note->channel, note->noteNo, note->velocity);
                } else {
                    SMFWriterAppendNoteOn(writer, note->tick, note->channel, note->noteNo, note->velocity);
                }
            }
            break;
        case MidiEventTypeTime:
            {
                TimeEvent *time = (TimeEvent *)events[i];
                SMFWriterAppendTime(writer, time->tick, time->numerator, time->denominator);
            }
            break;
        case MidiEventTypeKey:
            {
                KeyEvent *key = (KeyEvent *)events[i];
                uint8_t sf, mi;
                KeySignGetMidiExpression(key->keySign, &sf, &mi);
                SMFWriterAppendKey(writer, key->tick, sf, mi);
            }
            break;
        case MidiEventTypeTempo:
            {
                TempoEvent *tempo = (TempoEvent *)events[i];
                SMFWriterAppendTempo(writer, tempo->tick, tempo->tempo);
            }
            break;
        case MidiEventTypeMarker:
            {
                MarkerEvent *marker = (MarkerEvent *)events[i];
                SMFWriterAppendMarker(writer, marker->tick, marker->text);
            }
            break;
        case MidiEventTypeVoice:
            {
                VoiceEvent *voice = (VoiceEvent *)events[i];
                SMFWriterAppendControlChange(writer, voice->tick, voice->channel, 0x00, voice->msb);
                SMFWriterAppendControlChange(writer, voice->tick, voice->channel, 0x20, voice->lsb);
                SMFWriterAppendProgramChange(writer, voice->tick, voice->channel, voice->programNo);
            }
            break;
        case MidiEventTypeVolume:
            {
                VolumeEvent *event = (VolumeEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 7, event->value);
            }
            break;
        case MidiEventTypePan:
            {
                PanEvent *event = (PanEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 10, event->value);
            }
            break;
        case MidiEventTypeChorus:
            {
                ChorusEvent *event = (ChorusEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 93, event->value);
            }
            break;
        case MidiEventTypeReverb:
            {
                ReverbEvent *event = (ReverbEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 91, event->value);
            }
            break;
        }
    }

    bool ret = SMFWriterSerialize(writer);
    SMFWriterDestroy(writer);
    return ret;
}

bool ExporterWriteToWave(Exporter *self, const char *filepath)
{
    WaveWriter *writer = WaveWriterCreate();
    if (!WaveWriterOpenFile(writer, filepath)) {
        WaveWriterDestroy(writer);
        return false;
    }

    ExporterBuildEventsToWrite(self);

    ExporterAudioBuffer *audioBuffer = ExporterAudioBufferCreate();
    Mixer *mixer = MixerCreate((AudioOut *)audioBuffer);

    int count = NAArrayCount(self->eventsToWrite);
    MidiEvent **events = NAArrayGetValues(self->eventsToWrite);

    // TODO

    bool ret = WaveWriterSerialize(writer);

    MixerDestroy(mixer);
    ExporterAudioBufferDestroy(audioBuffer);
    WaveWriterDestroy(writer);

    return ret;
}


static double ExporterAudioBufferGetSampleRate(AudioOut *self);
static void ExporterAudioBufferRegisterCallback(AudioOut *self, AudioCallback function, void *receiver);
static void ExporterAudioBufferUnregisterCallback(AudioOut *self, AudioCallback function, void *receiver);

static ExporterAudioBuffer *ExporterAudioBufferCreate()
{
    ExporterAudioBuffer *self = calloc(1, sizeof(ExporterAudioBuffer));
    self->audioOut.getSampleRate = ExporterAudioBufferGetSampleRate;
    self->audioOut.registerCallback = ExporterAudioBufferRegisterCallback;
    self->audioOut.unregisterCallback = ExporterAudioBufferUnregisterCallback;
    return self;
}

static void ExporterAudioBufferDestroy(ExporterAudioBuffer *self)
{
    free(self);
}

static double ExporterAudioBufferGetSampleRate(AudioOut *self)
{
    return 44100.0;
}

static void ExporterAudioBufferRegisterCallback(AudioOut *_self, AudioCallback function, void *receiver)
{
    ExporterAudioBuffer *self = (ExporterAudioBuffer *)_self;
    self->callback = function;
    self->receiver = receiver;
}

static void ExporterAudioBufferUnregisterCallback(AudioOut *_self, AudioCallback function, void *receiver)
{
    ExporterAudioBuffer *self = (ExporterAudioBuffer *)_self;
    self->callback = NULL;
    self->receiver = NULL;
}
