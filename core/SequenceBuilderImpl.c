#include "SequenceBuilderImpl.h"
#include "Sequence.h"

#include <stdlib.h>
#include <string.h>

typedef struct _SequenceBuilderImpl {
    SequenceBuilder interface;
    Sequence *sequence;
} SequenceBuilderImpl;

static void SequenceBuilderDestroy(void *_self)
{
    SequenceBuilderImpl *self = _self;
    if (self->sequence) {
        SequenceRelease(self->sequence);
    }
    free(self);
}

static void SequenceBuilderSetResolution(void *self, int resolution)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    TimeTableSetResolution(sequence->timeTable, resolution);
}

static void SequenceBuilderSetTitle(void *self, const char *title)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    if (sequence->title) {
        free(sequence->title);
    }
    sequence->title = strdup(title);
}

static void SequenceBuilderAppendTempo(void *self, int id, int tick, float tempo)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    TempoEvent *event = MidiEventAlloc(MidiEventTypeTempo, id, tick, sizeof(TempoEvent) - sizeof(MidiEvent));
    event->tempo = tempo;
    NAArrayAppend(sequence->events, event);
    TimeTableAddTempo(sequence->timeTable, tick, tempo);
}

static void SequenceBuilderAppendTimeSign(void *self, int id, int tick, int numerator, int denominator)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    TimeEvent *event = MidiEventAlloc(MidiEventTypeTime, id, tick, sizeof(TimeEvent) - sizeof(MidiEvent));
    event->numerator = numerator;
    event->denominator = denominator;
    NAArrayAppend(sequence->events, event);
    TimeTableAddTimeSign(sequence->timeTable, tick, (TimeSign){numerator, denominator});
}

static void SequenceBuilderAppendKey(void *self, int id, int tick, int sf, int mi)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    KeyEvent *event = MidiEventAlloc(MidiEventTypeKey, id, tick, sizeof(KeyEvent) - sizeof(MidiEvent));
    event->sf = sf;
    event->mi = mi;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendNote(void *self, int id, int tick, int channel, int noteNo, int gatetime, int velocity)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    NoteEvent *event = MidiEventAlloc(MidiEventTypeNote, id, tick, sizeof(NoteEvent) - sizeof(MidiEvent));
    event->noteNo = noteNo;
    event->channel = channel;
    event->gatetime = gatetime;
    event->velocity = velocity;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendMarker(void *self, int id, int tick, const char *marker)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    MarkerEvent *event = MidiEventAlloc(MidiEventTypeMarker, id, tick, strlen(marker) + 1);
    strcpy(event->text, marker);
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendVoice(void *self, int id, int tick, int channel, int msb, int lsb, int programNo)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    VoiceEvent *event = MidiEventAlloc(MidiEventTypeVoice, id, tick, sizeof(VoiceEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->msb = msb;
    event->lsb = lsb;
    event->programNo = programNo;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendSynth(void *self, int id, int tick, int channel, const char *identifier)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    SynthEvent *event = MidiEventAlloc(MidiEventTypeSynth, id, tick, strlen(identifier) + 1);
    event->channel = channel;
    strcpy(event->identifier, identifier);
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendVolume(void *self, int id, int tick, int channel, int value)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    VolumeEvent *event = MidiEventAlloc(MidiEventTypeVolume, id, tick, sizeof(VolumeEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendPan(void *self, int id, int tick, int channel, int value)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    PanEvent *event = MidiEventAlloc(MidiEventTypePan, id, tick, sizeof(PanEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendChorus(void *self, int id, int tick, int channel, int value)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    ChorusEvent *event = MidiEventAlloc(MidiEventTypeChorus, id, tick, sizeof(ChorusEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendReverb(void *self, int id, int tick, int channel, int value)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;

    ReverbEvent *event = MidiEventAlloc(MidiEventTypeReverb, id, tick, sizeof(ReverbEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderSetLength(void *self, int length)
{
    Sequence *sequence = ((SequenceBuilderImpl *)self)->sequence;
    TimeTableSetLength(sequence->timeTable, length);
}

static void *SequenceBuilderBuild(void *_self)
{
    SequenceBuilderImpl *self = _self;
    SequenceSortEvents(self->sequence);
    Sequence *ret = self->sequence;
    self->sequence = NULL;
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
    self->interface.build = SequenceBuilderBuild;

    self->sequence = SequenceCreate();
    return (SequenceBuilder *)self;
}
