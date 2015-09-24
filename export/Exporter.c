#include "Exporter.h"
#include "SMFWriter.h"
#include "AudioOut.h"
#include "Mixer.h"
#include "Define.h"
#include "WaveWriter.h"
#include "NAArray.h"
#include "NASet.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define AUDIO_BUFFER_SIZE 64

struct _Exporter {
    Sequence *sequence;
    NASet *noteOffEvents;
    NAArray *eventsToWrite;
};

typedef struct _ExporterAudioBuffer {
    AudioOut audioOut;
    void *receiver;
    AudioCallback callback;
    AudioSample buffer[AUDIO_BUFFER_SIZE];
    double sampleRate;
} ExporterAudioBuffer;

static ExporterAudioBuffer *ExporterAudioBufferCreate(double sampleRate);
static void ExporterAudioBufferDestroy(ExporterAudioBuffer *self);
static double ExporterAudioBufferGetSampleRate(AudioOut *self);
static void ExporterAudioBufferRegisterCallback(AudioOut *self, AudioCallback function, void *receiver);
static void ExporterAudioBufferUnregisterCallback(AudioOut *self, AudioCallback function, void *receiver);


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

            NoteEvent *noteOff = MidiEventAlloc(MidiEventTypeNote, noteOn->id, noteOn->tick + noteOn->gatetime, sizeof(NoteEvent) - sizeof(MidiEvent));
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
                int value = Clip(event->value + 64, 0, 127);
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 10, value);
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
        case MidiEventTypeSynth:
            // not support
            break;
        }
    }

    bool ret = SMFWriterSerialize(writer);
    SMFWriterDestroy(writer);
    return ret;
}

static void ExporterBuildAudioSample(Exporter *self, ExporterAudioBuffer *audioBuffer, void (*callback)(Exporter *, ExporterAudioBuffer *, void *), void *context)
{
    Mixer *mixer = MixerCreate((AudioOut *)audioBuffer);

    NAArray *noteOffEvents = NAArrayCreate(4, NULL);
    double usecPerSample = 1.0 / ExporterAudioBufferGetSampleRate((AudioOut *)audioBuffer) * 1000.0 * 1000.0;
    int32_t length = TimeTableLength(self->sequence->timeTable);

    int bufferCount = 1;
    int64_t prevUsec = 0;
    int32_t tick = 0;
    int index = 0;

    while (tick < length) {
        int64_t usec = usecPerSample * AUDIO_BUFFER_SIZE * bufferCount;

        int32_t prevTick = TimeTableMicroSec2Tick(self->sequence->timeTable, prevUsec);
        tick = TimeTableMicroSec2Tick(self->sequence->timeTable, usec);

        for (int i = NAArrayCount(noteOffEvents) - 1; 0 <= i; --i) {
            NoteEvent *event = NAArrayGetValueAt(noteOffEvents, i);
            int offTick = event->tick + event->gatetime;
            if (prevTick <= offTick && offTick < tick) {
                MixerSendNoteOff(mixer, event);
                NAArrayRemoveAt(noteOffEvents, i);
            }
        }

        int count = NAArrayCount(self->eventsToWrite);
        MidiEvent **events = NAArrayGetValues(self->eventsToWrite);
        for (; index < count; ++index) {
            MidiEvent *event = events[index];
            if (prevTick <= event->tick && event->tick < tick) {
                switch (event->type) {
                case MidiEventTypeNote:
                    MixerSendNoteOn(mixer, (NoteEvent *)event);
                    NAArrayInsertAt(noteOffEvents, 0, event);
                    break;
                case MidiEventTypeVoice:
                    MixerSendVoice(mixer, (VoiceEvent *)event);
                    break;
                case MidiEventTypeVolume:
                    MixerSendVolume(mixer, (VolumeEvent *)event);
                    break;
                case MidiEventTypePan:
                    MixerSendPan(mixer, (PanEvent *)event);
                    break;
                case MidiEventTypeChorus:
                    MixerSendChorus(mixer, (ChorusEvent *)event);
                    break;
                case MidiEventTypeReverb:
                    MixerSendReverb(mixer, (ReverbEvent *)event);
                    break;
                case MidiEventTypeSynth:
                    MixerSendSynth(mixer, (SynthEvent *)event);
                    break;
                default:
                    break;
                }
            }
            else if (tick <= event->tick) {
                break;
            }
        }

        memset(audioBuffer->buffer, 0, sizeof(audioBuffer->buffer));
        audioBuffer->callback(audioBuffer->receiver, audioBuffer->buffer, AUDIO_BUFFER_SIZE);
        callback(self, audioBuffer, context);

        prevUsec = usec;
        ++bufferCount;
    }

    NAArrayDestroy(noteOffEvents);
    MixerDestroy(mixer);
}

static void ExporterBuildAudioSampleCallbackWave(Exporter *self, ExporterAudioBuffer *audioBuffer, void *context)
{
    WaveWriter *writer = context;
    int16_t samples[AUDIO_BUFFER_SIZE][2];

    for (int i = 0; i < AUDIO_BUFFER_SIZE; ++i) {
        int32_t L = round(0.0f <= audioBuffer->buffer[i].L
                ? (double)audioBuffer->buffer[i].L * 32767.0
                : (double)audioBuffer->buffer[i].L * 32768.0);
        int32_t R = round(0.0f <= audioBuffer->buffer[i].R
                ? (double)audioBuffer->buffer[i].R * 32767.0
                : (double)audioBuffer->buffer[i].R * 32768.0);

        samples[i][0] = Clip(L, -32768, 32767);
        samples[i][1] = Clip(R, -32768, 32767);
    }

    WaveWriterAppendData(writer, (int32_t *)samples, AUDIO_BUFFER_SIZE);
}

bool ExporterWriteToWave(Exporter *self, const char *filepath)
{
    WaveWriter *writer = WaveWriterCreate();
    if (!WaveWriterOpenFile(writer, filepath)) {
        WaveWriterDestroy(writer);
        return false;
    }

    ExporterBuildEventsToWrite(self);

    ExporterAudioBuffer *audioBuffer = ExporterAudioBufferCreate(44100.0);
    ExporterBuildAudioSample(self, audioBuffer, ExporterBuildAudioSampleCallbackWave, writer);
    ExporterAudioBufferDestroy(audioBuffer);

    bool ret = WaveWriterSerialize(writer, 44100.0);

    WaveWriterDestroy(writer);
    return ret;
}


static ExporterAudioBuffer *ExporterAudioBufferCreate(double sampleRate)
{
    ExporterAudioBuffer *self = calloc(1, sizeof(ExporterAudioBuffer));
    self->audioOut.getSampleRate = ExporterAudioBufferGetSampleRate;
    self->audioOut.registerCallback = ExporterAudioBufferRegisterCallback;
    self->audioOut.unregisterCallback = ExporterAudioBufferUnregisterCallback;
    self->sampleRate = sampleRate;
    return self;
}

static void ExporterAudioBufferDestroy(ExporterAudioBuffer *self)
{
    if (self->callback && self->receiver) {
        self->callback(self->receiver, self->buffer, AUDIO_BUFFER_SIZE);
    }

    free(self);
}

static double ExporterAudioBufferGetSampleRate(AudioOut *_self)
{
    ExporterAudioBuffer *self = (ExporterAudioBuffer *)_self;
    return self->sampleRate;
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
