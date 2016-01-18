#include "Exporter.h"
#include "Sequence.h"
#include "SequenceBuilderImpl.h"
#include "SMFWriter.h"
#include "AudioOut.h"
#include "Mixer.h"
#include "Define.h"
#include "WaveWriter.h"
#include "AACWriter.h"
#include "NAArray.h"
#include "NASet.h"
#include "NAIO.h"
#include "NALog.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#define AUDIO_BUFFER_SIZE 64

struct _Exporter {
    Sequence *sequence;
    NASet *noteOffEvents;
    NAArray *eventsToWrite;
    ExporterObserverCallbacks *callbacks;
    void *receiver;
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


Exporter *ExporterCreate(ExporterObserverCallbacks *callbacks, void *receiver)
{
    Exporter *self = calloc(1, sizeof(Exporter));
    self->callbacks = callbacks;
    self->receiver = receiver;
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

    free(self);
}

static bool ExporterWriteToSMF(Exporter *self, const char *filepath);
static bool ExporterWriteToWave(Exporter *self, const char *filepath);
static bool ExporterWriteToAAC(Exporter *self, const char *filepath);

static bool HasSoundSource()
{
    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    NAArray *descriptions = MidiSourceManagerGetAvailableDescriptions(manager);
    return 0 < NAArrayCount(descriptions);
}

ExporterError ExporterExport(Exporter *self, const char *filepath, const char *output)
{
    const struct {
        const char *ext;
        bool (*function)(Exporter *, const char *);
        bool needSoundSource;
    } table[] = {
        {"mid", ExporterWriteToSMF, false},
        {"midi", ExporterWriteToSMF, false},
        {"smf", ExporterWriteToSMF, false},
        {"wav", ExporterWriteToWave, true},
        {"wave", ExporterWriteToWave, true},
        {"m4a", ExporterWriteToAAC, true},
        {"aac", ExporterWriteToAAC, true},
    };

    bool (*function)(Exporter *, const char *) = NULL;
    const char *ext = NAIOGetFileExtenssion(output);

    for (int i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
        if (0 == strcasecmp(table[i].ext, ext)) {
            function = table[i].function;
            if (table[i].needSoundSource && !HasSoundSource()) {
                return ExporterErrorNoSoundSource;
            }
            break;
        }
    }

    if (!function) {
        return ExporterErrorUnsupportedFileType;
    }

    ExporterError ret = ExporterErrorNoError;

    SequenceBuilder *builder = SequenceBuilderCreate();
    Parser *parser = ParserCreate(builder);

    ParseInfo *info = NULL;
    Sequence *sequence = ParserParseFile(parser, filepath, &info);

    ParserDestroy(parser);
    builder->destroy(builder);

    self->callbacks->onParseFinish(self->receiver, info);

    self->sequence = sequence;

    if (!function(self, output)) {
        ret = ExporterErrorCouldNotWriteFile;
    }

EXIT:
    SequenceRelease(sequence);
    ParseInfoRelease(info);

    return ret;
}

static void ExporterBuildEventsToWrite(Exporter *self)
{
    int count = NAArrayCount(self->sequence->events);
    self->eventsToWrite = NAArrayCreate(((count * 2 / 1024) + 1) * 1024, NULL);
    self->noteOffEvents = NASetCreate(NAHashAddress, NULL);

    NAIterator *iterator = NAArrayGetIterator(self->sequence->events);
    while (iterator->hasNext(iterator)) {
        MidiEvent *event = iterator->next(iterator);
        NAArrayAppend(self->eventsToWrite, event);

        if (MidiEventTypeNote == event->type) {
            NoteEvent *noteOn = (NoteEvent *)event;

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

    int progress = -1;

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
                SMFWriterAppendKey(writer, key->tick, key->sf, key->mi);
            }
            break;
        case MidiEventTypeTempo:
            {
                TempoEvent *tempo = (TempoEvent *)events[i];
                SMFWriterAppendTempo(writer, tempo->tick, tempo->tempo);
            }
            break;
        case MidiEventTypeTitle:
            {
                TitleEvent *title = (TitleEvent *)events[i];
                SMFWriterAppendTitle(writer, title->tick, title->text);
            }
            break;
        case MidiEventTypeCopyright:
            {
                CopyrightEvent *copyright = (CopyrightEvent *)events[i];
                SMFWriterAppendCopyright(writer, copyright->tick, copyright->text);
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
        case MidiEventTypeExpression:
            {
                ExpressionEvent *event = (ExpressionEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 11, event->value);
            }
            break;
        case MidiEventTypeDetune:
            {
                DetuneEvent *event = (DetuneEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 101, 0);
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 100, 1);
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 6, event->fine.msb);
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 38, event->fine.lsb);
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 101, 0);
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 100, 2);
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 6, event->corse.msb);
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 101, 0x7F);
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 100, 0x7F);
            }
            break;
        case MidiEventTypeSynth:
            // not support
            break;
        }

        int _progress = i * 100 / (count - 1);
        if (progress != _progress) {
            progress = _progress;
            self->callbacks->onProgress(self->receiver, progress);
        }
    }

    bool ret = SMFWriterSerialize(writer);
    SMFWriterDestroy(writer);
    return ret;
}

static void ExporterBuildAudioSample(Exporter *self, ExporterAudioBuffer *audioBuffer, void (*callback)(Exporter *, int32_t *, int, void *), void *context)
{
    Mixer *mixer = MixerCreate((AudioOut *)audioBuffer);

    double usecPerSample = 1.0 / ExporterAudioBufferGetSampleRate((AudioOut *)audioBuffer) * 1000.0 * 1000.0;
    int32_t length = TimeTableLength(self->sequence->timeTable);

    int bufferCount = 1;
    int64_t prevUsec = 0;
    int32_t tick = 0;
    int index = 0;
    int progress = -1;

    int16_t samples[AUDIO_BUFFER_SIZE][2];

    while (tick < length) {
        int64_t usec = usecPerSample * AUDIO_BUFFER_SIZE * bufferCount;

        if (__IsTrace__) {
            int sec = usec / 1000000;
            struct timeval tv;
            gettimeofday(&tv, NULL);
            printf("%d%% %d:%02d now=%ld\n", (tick * 100) / length, sec / 60, sec % 60, tv.tv_sec);
        }

        int32_t prevTick = TimeTableMicroSec2Tick(self->sequence->timeTable, prevUsec);
        tick = TimeTableMicroSec2Tick(self->sequence->timeTable, usec);

        int count = NAArrayCount(self->eventsToWrite);
        MidiEvent **events = NAArrayGetValues(self->eventsToWrite);
        for (; index < count; ++index) {
            MidiEvent *event = events[index];
            if (prevTick <= event->tick && event->tick < tick) {
                switch (event->type) {
                case MidiEventTypeNote:
                    if (NASetContains(self->noteOffEvents, event)) {
                        MixerSendNoteOff(mixer, (NoteEvent *)event);
                    } else {
                        MixerSendNoteOn(mixer, (NoteEvent *)event);
                    }
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

        callback(self, (int32_t *)samples, AUDIO_BUFFER_SIZE, context);

        prevUsec = usec;
        ++bufferCount;

        int _progress = tick * 100 / (length - 1);
        if (progress != _progress) {
            progress = _progress;
            self->callbacks->onProgress(self->receiver, progress);
        }
    }

    MixerDestroy(mixer);
}

static void ExporterBuildAudioSampleCallbackWave(Exporter *self, int32_t *data, int count, void *context)
{
    WaveWriter *writer = context;
    WaveWriterAppendData(writer, data, count);
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

static void ExporterBuildAudioSampleCallbackAAC(Exporter *self, int32_t *data, int count, void *context)
{
    AACWriter *writer = context;
    AACWriterAppendData(writer, data, count);
}

bool ExporterWriteToAAC(Exporter *self, const char *filepath)
{
    AACWriter *writer = AACWriterCreate(44100.0);
    if (!AACWriterOpenFile(writer, filepath)) {
        AACWriterDestroy(writer);
        return false;
    }

    ExporterBuildEventsToWrite(self);

    ExporterAudioBuffer *audioBuffer = ExporterAudioBufferCreate(44100.0);
    ExporterBuildAudioSample(self, audioBuffer, ExporterBuildAudioSampleCallbackAAC, writer);
    ExporterAudioBufferDestroy(audioBuffer);

    bool ret = AACWriterSerialize(writer);

    AACWriterDestroy(writer);
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
