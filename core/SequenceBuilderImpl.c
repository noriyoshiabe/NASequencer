#include "SequenceBuilderImpl.h"
#include "Sequence.h"

#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

typedef struct _SequenceBuilderImpl {
    SequenceBuilder interface;
    Sequence *sequence;
    int id;
} SequenceBuilderImpl;

static void SequenceBuilderDestroy(void *_self)
{
    SequenceBuilderImpl *self = _self;
    
    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    free(self);
}

static void SequenceBuilderSetResolution(void *_self, int resolution)
{
    SequenceBuilderImpl *self = _self;
    TimeTableSetResolution(self->sequence->timeTable, resolution);
}

static void SequenceBuilderAppendTitle(void *_self, int tick, const char *title)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    if (!sequence->title) {
        sequence->title = strdup(title);
    }

    TitleEvent *titleEvent = MidiEventAlloc(MidiEventTypeTitle, ++self->id, tick, strlen(title) + 1);
    strcpy(titleEvent->text, title);
    NAArrayAppend(sequence->events, titleEvent);
}

static void SequenceBuilderAppendCopyright(void *_self, int tick, const char *text)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    CopyrightEvent *copyrightEvent = MidiEventAlloc(MidiEventTypeCopyright, ++self->id, tick, strlen(text) + 1);
    strcpy(copyrightEvent->text, text);
    NAArrayAppend(sequence->events, copyrightEvent);
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
    TimeTableAddRepeatPoint(sequence->timeTable, tick);
}

static void SequenceBuilderAppendBank(void *_self, int tick, int channel, int bankNo)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    BankEvent *event = MidiEventAlloc(MidiEventTypeBank, ++self->id, tick, sizeof(BankEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->bankNo = bankNo;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendProgram(void *_self, int tick, int channel, int programNo)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    ProgramEvent *event = MidiEventAlloc(MidiEventTypeProgram, ++self->id, tick, sizeof(ProgramEvent) - sizeof(MidiEvent));
    event->channel = channel;
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

static void SequenceBuilderAppendExpression(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    ExpressionEvent *event = MidiEventAlloc(MidiEventTypeExpression, ++self->id, tick, sizeof(ExpressionEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendPitch(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    PitchEvent *event = MidiEventAlloc(MidiEventTypePitch, ++self->id, tick, sizeof(PitchEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendDetune(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    DetuneEvent *event = MidiEventAlloc(MidiEventTypeDetune, ++self->id, tick, sizeof(DetuneEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;

    int sign = event->value / MAX(1, abs(event->value));
    int fine = (int)((double)(abs(event->value) % 100 * sign) / (100.0 / 8192.0)) + 8192;
    int corse = (abs(event->value) / 100 * sign) + 64;

    event->fine.msb = 0x7F & (fine >> 7);
    event->fine.lsb = 0x7F & fine;
    event->corse.msb = corse;

    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendPitchSense(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    PitchSenseEvent *event = MidiEventAlloc(MidiEventTypePitchSense, ++self->id, tick, sizeof(PitchSenseEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(sequence->events, event);
}

static void SequenceBuilderAppendSustain(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    Sequence *sequence = self->sequence;

    SustainEvent *event = MidiEventAlloc(MidiEventTypeSustain, ++self->id, tick, sizeof(SustainEvent) - sizeof(MidiEvent));
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
    self->interface.appendTitle = SequenceBuilderAppendTitle;
    self->interface.appendCopyright = SequenceBuilderAppendCopyright;
    self->interface.appendTempo = SequenceBuilderAppendTempo;
    self->interface.appendTimeSign = SequenceBuilderAppendTimeSign;
    self->interface.appendKey = SequenceBuilderAppendKey;
    self->interface.appendNote = SequenceBuilderAppendNote;
    self->interface.appendMarker = SequenceBuilderAppendMarker;
    self->interface.appendBank = SequenceBuilderAppendBank;
    self->interface.appendProgram = SequenceBuilderAppendProgram;
    self->interface.appendSynth = SequenceBuilderAppendSynth;
    self->interface.appendVolume = SequenceBuilderAppendVolume;
    self->interface.appendPan = SequenceBuilderAppendPan;
    self->interface.appendChorus = SequenceBuilderAppendChorus;
    self->interface.appendReverb = SequenceBuilderAppendReverb;
    self->interface.appendExpression = SequenceBuilderAppendExpression;
    self->interface.appendPitch = SequenceBuilderAppendPitch;
    self->interface.appendSustain = SequenceBuilderAppendSustain;
    self->interface.appendDetune = SequenceBuilderAppendDetune;
    self->interface.appendPitchSense = SequenceBuilderAppendPitchSense;
    self->interface.setLength = SequenceBuilderSetLength;
    self->interface.build = SequenceBuilderBuild;

    self->sequence = SequenceCreate();

    return (SequenceBuilder *)self;
}
