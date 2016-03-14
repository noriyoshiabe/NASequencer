#include "MidiEvent.h"

#include <stdio.h>
#include <stdlib.h>

void *MidiEventAlloc(MidiEventType type, int id, int tick, int extraSize)
{
    int size;

    switch (type) {
    case MidiEventTypeNote:
        size = sizeof(NoteEvent);
        break;
    case MidiEventTypeTempo:
        size = sizeof(TempoEvent);
        break;
    case MidiEventTypeTime:
        size = sizeof(TimeEvent);
        break;
    case MidiEventTypeKey:
        size = sizeof(KeyEvent);
        break;
    case MidiEventTypeTitle:
        size = sizeof(TitleEvent) + extraSize;
        break;
    case MidiEventTypeCopyright:
        size = sizeof(CopyrightEvent) + extraSize;
        break;
    case MidiEventTypeMarker:
        size = sizeof(MarkerEvent) + extraSize;
        break;
    case MidiEventTypeBank:
        size = sizeof(BankEvent);
        break;
    case MidiEventTypeProgram:
        size = sizeof(ProgramEvent);
        break;
    case MidiEventTypeVolume:
        size = sizeof(VolumeEvent);
        break;
    case MidiEventTypePan:
        size = sizeof(PanEvent);
        break;
    case MidiEventTypeChorus:
        size = sizeof(ChorusEvent);
        break;
    case MidiEventTypeReverb:
        size = sizeof(ReverbEvent);
        break;
    case MidiEventTypeExpression:
        size = sizeof(ExpressionEvent);
        break;
    case MidiEventTypeDetune:
        size = sizeof(DetuneEvent);
        break;
    case MidiEventTypeSynth:
        size = sizeof(SynthEvent) + extraSize;
        break;
    }

    MidiEvent *ret = calloc(1, size);
    ret->type = type;
    ret->id = id;
    ret->tick = tick;
    return ret;
}

void MidiEventDump(MidiEvent *self, int indent)
{
    printf("%*s", indent, "");
    void *_self = self;

    switch (self->type) {
    case MidiEventTypeNote:
        {
            NoteEvent *self = _self;
            printf("Note: id=%d tick=%d channel=%d noteNo=%d gatetime=%d velocity=%d\n",
                    self->id, self->tick, self->channel, self->noteNo, self->gatetime, self->velocity);
        }
        break;
    case MidiEventTypeTempo:
        {
            TempoEvent *self = _self;
            printf("Tempo: id=%d tick=%d tempo=%f\n",
                    self->id, self->tick, self->tempo);
        }
        break;
    case MidiEventTypeTime:
        {
            TimeEvent *self = _self;
            printf("Time: id=%d tick=%d time=%d/%d\n",
                    self->id, self->tick, self->numerator, self->denominator);
        }
        break;
    case MidiEventTypeKey:
        {
            KeyEvent *self = _self;
            printf("Key: id=%d tick=%d sf=%d mi=%d\n",
                    self->id, self->tick, self->sf, self->mi);
        }
        break;
    case MidiEventTypeTitle:
        {
            TitleEvent *self = _self;
            printf("Title: id=%d tick=%d text=%s\n",
                    self->id, self->tick, self->text);
        }
        break;
    case MidiEventTypeCopyright:
        {
            CopyrightEvent *self = _self;
            printf("Copyright: id=%d tick=%d text=%s\n",
                    self->id, self->tick, self->text);
        }
        break;
    case MidiEventTypeMarker:
        {
            MarkerEvent *self = _self;
            printf("Marker: id=%d tick=%d text=%s\n",
                    self->id, self->tick, self->text);
        }
        break;
    case MidiEventTypeBank:
        {
            BankEvent *self = _self;
            printf("Bank: id=%d tick=%d channel=%d bankNo=%d\n",
                    self->id, self->tick, self->channel, self->bankNo);
        }
        break;
    case MidiEventTypeProgram:
        {
            ProgramEvent *self = _self;
            printf("Program: id=%d tick=%d channel=%d programNo=%d\n",
                    self->id, self->tick, self->channel, self->programNo);
        }
        break;
    case MidiEventTypeVolume:
        {
            VolumeEvent *self = _self;
            printf("Volume: id=%d tick=%d channel=%d value=%d\n",
                    self->id, self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypePan:
        {
            PanEvent *self = _self;
            printf("Pan: id=%d tick=%d channel=%d value=%d\n",
                    self->id, self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeChorus:
        {
            ChorusEvent *self = _self;
            printf("Chorus: id=%d tick=%d channel=%d value=%d\n",
                    self->id, self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeReverb:
        {
            ReverbEvent *self = _self;
            printf("Reverb: id=%d tick=%d channel=%d value=%d\n",
                    self->id, self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeExpression:
        {
            ExpressionEvent *self = _self;
            printf("Synth: id=%d tick=%d value=%d\n",
                    self->id, self->tick, self->value);
        }
        break;
    case MidiEventTypeDetune:
        {
            DetuneEvent *self = _self;
            printf("Synth: id=%d tick=%d value=%d\n",
                    self->id, self->tick, self->value);
        }
        break;
    case MidiEventTypeSynth:
        {
            SynthEvent *self = _self;
            printf("Synth: id=%d tick=%d identifier=%s\n",
                    self->id, self->tick, self->identifier);
        }
        break;
    }
}

int MidiEventGetChannel(const MidiEvent *event)
{
    switch (event->type) {
    case MidiEventTypeNote:
    case MidiEventTypeBank:
    case MidiEventTypeProgram:
    case MidiEventTypeVolume:
    case MidiEventTypePan:
    case MidiEventTypeChorus:
    case MidiEventTypeReverb:
    case MidiEventTypeSynth:
    case MidiEventTypeExpression:
    case MidiEventTypeDetune:
        return ((ChannelEvent *)event)->channel;
    default:
        return 0;
    }
}

int MidiEventComparator(const void *_event1, const void *_event2)
{
    const MidiEvent *event1 = *((const MidiEvent **)_event1);
    const MidiEvent *event2 = *((const MidiEvent **)_event2);

    int result;
    
    result = event1->tick - event2->tick;
    if (0 != result) {
        return result;
    }

    result = MidiEventGetChannel(event1) - MidiEventGetChannel(event2);
    if (0 != result) {
        return result;
    }

    return event1->id - event2->id;
}

int MidiEventIDComparator(const void *event1, const void *event2)
{
    return ((const MidiEvent *)event1)->id - ((const MidiEvent *)event2)->id;
}

