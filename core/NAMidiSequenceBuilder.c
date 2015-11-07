#include "NAMidiSequenceBuilder.h"
#include "Sequence.h"

#include <stdlib.h>
#include <string.h>

typedef struct _NAMidiSequenceBuilder {
    SequenceBuilder interface;
    Sequence *sequence;
} NAMidiSequenceBuilder;

static void NAMidiSequenceBuilderDestroy(void *_self)
{
    NAMidiSequenceBuilder *self = _self;
    if (self->sequence) {
        SequenceRelease(self->sequence);
    }
    free(self);
}

static void NAMidiSequenceBuilderSetResolution(void *self, int resolution)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    TimeTableSetResolution(sequence->timeTable, resolution);
}

static void NAMidiSequenceBuilderSetTitle(void *self, const char *title)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    if (sequence->title) {
        free(sequence->title);
    }
    sequence->title = strdup(title);
}

static void NAMidiSequenceBuilderAppendTempo(void *self, int id, int tick, float tempo)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    TempoEvent *event = MidiEventAlloc(MidiEventTypeTempo, id, tick, sizeof(TempoEvent) - sizeof(MidiEvent));
    event->tempo = tempo;
    NAArrayAppend(sequence->events, event);
    TimeTableAddTempo(sequence->timeTable, tick, tempo);
}

static void NAMidiSequenceBuilderAppendTimeSign(void *self, int id, int tick, int numerator, int denominator)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    TimeEvent *event = MidiEventAlloc(MidiEventTypeTime, id, tick, sizeof(TimeEvent) - sizeof(MidiEvent));
    event->numerator = numerator;
    event->denominator = denominator;
    NAArrayAppend(sequence->events, event);
    TimeTableAddTimeSign(sequence->timeTable, tick, (TimeSign){numerator, denominator});
}

static void NAMidiSequenceBuilderAppendNote(void *self, int id, int tick, int channel, int noteNo, int gatetime, int velocity)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    NoteEvent *event = MidiEventAlloc(MidiEventTypeNote, id, tick, sizeof(NoteEvent) - sizeof(MidiEvent));
    event->noteNo = noteNo;
    event->channel = channel;
    event->gatetime = gatetime;
    event->velocity = velocity;
    NAArrayAppend(sequence->events, event);
}

static void NAMidiSequenceBuilderAppendMarker(void *self, int id, int tick, const char *marker)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    MarkerEvent *event = MidiEventAlloc(MidiEventTypeMarker, id, tick, strlen(marker) + 1);
    strcpy(event->text, marker);
    NAArrayAppend(sequence->events, event);
}

static void NAMidiSequenceBuilderAppendVoice(void *self, int id, int tick, int channel, int msb, int lsb, int programNo)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    VoiceEvent *event = MidiEventAlloc(MidiEventTypeVoice, id, tick, sizeof(VoiceEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->msb = msb;
    event->lsb = lsb;
    event->programNo = programNo;
    NAArrayAppend(sequence->events, event);
}

static void NAMidiSequenceBuilderAppendSynth(void *self, int id, int tick, int channel, const char *identifier)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    SynthEvent *event = MidiEventAlloc(MidiEventTypeSynth, id, tick, strlen(identifier) + 1);
    event->channel = channel;
    strcpy(event->identifier, identifier);
    NAArrayAppend(sequence->events, event);
}

static void NAMidiSequenceBuilderAppendVolume(void *self, int id, int tick, int channel, int value)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    VolumeEvent *event = MidiEventAlloc(MidiEventTypeVolume, id, tick, sizeof(VolumeEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void NAMidiSequenceBuilderAppendPan(void *self, int id, int tick, int channel, int value)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    PanEvent *event = MidiEventAlloc(MidiEventTypePan, id, tick, sizeof(PanEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void NAMidiSequenceBuilderAppendChorus(void *self, int id, int tick, int channel, int value)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    ChorusEvent *event = MidiEventAlloc(MidiEventTypeChorus, id, tick, sizeof(ChorusEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void NAMidiSequenceBuilderAppendReverb(void *self, int id, int tick, int channel, int value)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;

    ReverbEvent *event = MidiEventAlloc(MidiEventTypeReverb, id, tick, sizeof(ReverbEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void NAMidiSequenceBuilderSetLength(void *self, int length)
{
    Sequence *sequence = ((NAMidiSequenceBuilder *)self)->sequence;
    TimeTableSetLength(sequence->timeTable, length);
}

static void *NAMidiSequenceBuilderBuild(void *_self)
{
    NAMidiSequenceBuilder *self = _self;
    SequenceSortEvents(self->sequence);
    Sequence *ret = self->sequence;
    self->sequence = NULL;
    return ret;
}

SequenceBuilder *NAMidiSequenceBuilderCreate()
{
    NAMidiSequenceBuilder *self = calloc(1, sizeof(NAMidiSequenceBuilder));

    self->interface.destroy = NAMidiSequenceBuilderDestroy;
    self->interface.setResolution = NAMidiSequenceBuilderSetResolution;
    self->interface.setTitle = NAMidiSequenceBuilderSetTitle;
    self->interface.appendTempo = NAMidiSequenceBuilderAppendTempo;
    self->interface.appendTimeSign = NAMidiSequenceBuilderAppendTimeSign;
    self->interface.appendNote = NAMidiSequenceBuilderAppendNote;
    self->interface.appendMarker = NAMidiSequenceBuilderAppendMarker;
    self->interface.appendVoice = NAMidiSequenceBuilderAppendVoice;
    self->interface.appendSynth = NAMidiSequenceBuilderAppendSynth;
    self->interface.appendVolume = NAMidiSequenceBuilderAppendVolume;
    self->interface.appendPan = NAMidiSequenceBuilderAppendPan;
    self->interface.appendChorus = NAMidiSequenceBuilderAppendChorus;
    self->interface.appendReverb = NAMidiSequenceBuilderAppendReverb;
    self->interface.setLength = NAMidiSequenceBuilderSetLength;
    self->interface.build = NAMidiSequenceBuilderBuild;

    self->sequence = SequenceCreate();
    return (SequenceBuilder *)self;
}
