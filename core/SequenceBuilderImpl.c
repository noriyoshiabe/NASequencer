#include "SequenceBuilderImpl.h"
#include "Sequence.h"
#include "BuildInformation.h"

#include <stdlib.h>
#include <string.h>

typedef struct _SequenceBuilderImpl {
    SequenceBuilder interface;
    Sequence *sequence;
    BuildInformation *info;
    int id;
    TitleEvent *titleEvent;
} SequenceBuilderImpl;

static void SequenceBuilderDestroy(void *_self)
{
    SequenceBuilderImpl *self = _self;
    
    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    if (self->info) {
        BuildInformationRelease(self->info);
    }

    free(self);
}

static void SequenceBuilderSetResolution(void *_self, int resolution)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    TimeTableSetResolution(self->sequence->timeTable, resolution);
}

static void SequenceBuilderSetTitle(void *_self, const char *title)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    if (sequence->title) {
        free(sequence->title);
    }
    sequence->title = strdup(title);

    if (self->titleEvent) {
        int index = NAArrayBSearchIndex(sequence->events, self->titleEvent, MidiEventIDComparator);
        NAArrayRemoveAt(sequence->events, index);
        free(self->titleEvent);
    }

    self->titleEvent = MidiEventAlloc(MidiEventTypeTitle, ++self->id, 0, strlen(title) + 1);
    strcpy(self->titleEvent->text, title);
    NAArrayAppend(sequence->events, self->titleEvent);
}

static void SequenceBuilderAppendTempo(void *_self, int tick, float tempo)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    TempoEvent *event = MidiEventAlloc(MidiEventTypeTempo, ++self->id, tick, sizeof(TempoEvent) - sizeof(MidiEvent));
    event->tempo = tempo;
    NAArrayAppend(sequence->events, event);
    TimeTableAddTempo(sequence->timeTable, tick, tempo);
}

static void SequenceBuilderAppendTimeSign(void *_self, int tick, int numerator, int denominator)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    TimeEvent *event = MidiEventAlloc(MidiEventTypeTime, ++self->id, tick, sizeof(TimeEvent) - sizeof(MidiEvent));
    event->numerator = numerator;
    event->denominator = denominator;
    NAArrayAppend(sequence->events, event);
    TimeTableAddTimeSign(sequence->timeTable, tick, (TimeSign){numerator, denominator});
}

static void SequenceBuilderAppendKey(void *_self, int tick, int sf, int mi)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    KeyEvent *event = MidiEventAlloc(MidiEventTypeKey, ++self->id, tick, sizeof(KeyEvent) - sizeof(MidiEvent));
    event->sf = sf;
    event->mi = mi;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendNote(void *_self, int tick, int channel, int noteNo, int gatetime, int velocity)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    NoteEvent *event = MidiEventAlloc(MidiEventTypeNote, ++self->id, tick, sizeof(NoteEvent) - sizeof(MidiEvent));
    event->noteNo = noteNo;
    event->channel = channel;
    event->gatetime = gatetime;
    event->velocity = velocity;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendMarker(void *_self, int tick, const char *marker)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    MarkerEvent *event = MidiEventAlloc(MidiEventTypeMarker, ++self->id, tick, strlen(marker) + 1);
    strcpy(event->text, marker);
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendVoice(void *_self, int tick, int channel, int msb, int lsb, int programNo)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    VoiceEvent *event = MidiEventAlloc(MidiEventTypeVoice, ++self->id, tick, sizeof(VoiceEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->msb = msb;
    event->lsb = lsb;
    event->programNo = programNo;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendSynth(void *_self, int tick, int channel, const char *identifier)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    SynthEvent *event = MidiEventAlloc(MidiEventTypeSynth, ++self->id, tick, strlen(identifier) + 1);
    event->channel = channel;
    strcpy(event->identifier, identifier);
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendVolume(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    VolumeEvent *event = MidiEventAlloc(MidiEventTypeVolume, ++self->id, tick, sizeof(VolumeEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendPan(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    PanEvent *event = MidiEventAlloc(MidiEventTypePan, ++self->id, tick, sizeof(PanEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendChorus(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    ChorusEvent *event = MidiEventAlloc(MidiEventTypeChorus, ++self->id, tick, sizeof(ChorusEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendReverb(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    ReverbEvent *event = MidiEventAlloc(MidiEventTypeReverb, ++self->id, tick, sizeof(ReverbEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderSetLength(void *_self, int length)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    TimeTableSetLength(sequence->timeTable, length);
}

static void SequenceBuilderAppendFilepath(void *_self, const char *filepath)
{
    SequenceBuilderImpl *self = _self;
    NAArrayAppend(self->info->filepaths, strdup(filepath));
}

static void SequenceBuilderAppendError(void *_self, void *_error)
{
    SequenceBuilderImpl *self = _self;
    ParseError *error = _error;
    ParseError *copy = malloc(sizeof(ParseError));
    memcpy(copy, error, sizeof(ParseError));
    copy->location.filepath = (char *)strdup(error->location.filepath);
    NAArrayAppend(self->info->errors, copy);
}

static void *SequenceBuilderBuildSequence(void *_self)
{
    SequenceBuilderImpl *self = _self;
    SequenceSortEvents(self->sequence);
    Sequence *ret = self->sequence;
    self->sequence = NULL;
    return ret;
}

static void *SequenceBuilderBuildInforamtion(void *_self)
{
    SequenceBuilderImpl *self = _self;
    BuildInformation *ret = self->info;
    self->info = NULL;
    return ret;
}

SequenceBuilder *SequenceBuilderCreate()
{
    SequenceBuilderImpl *self = calloc(1, sizeof(SequenceBuilderImpl));

    self->interface.destroy = SequenceBuilderDestroy;
    self->interface.setResolution = SequenceBuilderSetResolution;
    self->interface.setTitle = SequenceBuilderSetTitle;
    self->interface.appendTempo = SequenceBuilderAppendTempo;
    self->interface.appendTimeSign = SequenceBuilderAppendTimeSign;
    self->interface.appendKey = SequenceBuilderAppendKey;
    self->interface.appendNote = SequenceBuilderAppendNote;
    self->interface.appendMarker = SequenceBuilderAppendMarker;
    self->interface.appendVoice = SequenceBuilderAppendVoice;
    self->interface.appendSynth = SequenceBuilderAppendSynth;
    self->interface.appendVolume = SequenceBuilderAppendVolume;
    self->interface.appendPan = SequenceBuilderAppendPan;
    self->interface.appendChorus = SequenceBuilderAppendChorus;
    self->interface.appendReverb = SequenceBuilderAppendReverb;
    self->interface.setLength = SequenceBuilderSetLength;

    self->interface.appendFilepath = SequenceBuilderAppendFilepath;
    self->interface.appendError = SequenceBuilderAppendError;

    self->interface.buildSequence = SequenceBuilderBuildSequence;
    self->interface.buildInfomation = SequenceBuilderBuildInforamtion;

    self->sequence = SequenceCreate();
    self->info = BuildInformationCreate();

    return (SequenceBuilder *)self;
}
