#include "MidiEvent.h"

#include <stdio.h>
#include <stdlib.h>

void *MidiEventAlloc(MidiEventType type, int tick, int extraSize)
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
    case MidiEventTypeMarker:
        size = sizeof(MarkerEvent) + extraSize;
        break;
    case MidiEventTypeVoice:
        size = sizeof(VoiceEvent);
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
    }

    MidiEvent *ret = calloc(1, size);
    ret->type = type;
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
            printf("Note: tick=%d channel=%d noteNo=%d gatetime=%d velocity=%d\n",
                    self->tick, self->channel, self->noteNo, self->gatetime, self->velocity);
        }
        break;
    case MidiEventTypeTempo:
        {
            TempoEvent *self = _self;
            printf("Tempo: tick=%d tempo=%f\n",
                    self->tick, self->tempo);
        }
        break;
    case MidiEventTypeTime:
        {
            TimeEvent *self = _self;
            printf("Time: tick=%d time=%d/%d\n",
                    self->tick, self->numerator, self->denominator);
        }
        break;
    case MidiEventTypeKey:
        {
            KeyEvent *self = _self;
            printf("Key: tick=%d key=%s\n",
                    self->tick, KeySign2String(self->keySign));
        }
        break;
    case MidiEventTypeMarker:
        {
            MarkerEvent *self = _self;
            printf("Marker: tick=%d text=%s\n",
                    self->tick, self->text);
        }
        break;
    case MidiEventTypeVoice:
        {
            VoiceEvent *self = _self;
            printf("Voice: tick=%d channel=%d msb=%d lsb=%d programNo=%d\n",
                    self->tick, self->channel, self->msb, self->lsb, self->programNo);
        }
        break;
    case MidiEventTypeVolume:
        {
            VolumeEvent *self = _self;
            printf("Volume: tick=%d channel=%d value=%d\n",
                    self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypePan:
        {
            PanEvent *self = _self;
            printf("Pan: tick=%d channel=%d value=%d\n",
                    self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeChorus:
        {
            ChorusEvent *self = _self;
            printf("Chorus: tick=%d channel=%d value=%d\n",
                    self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeReverb:
        {
            ReverbEvent *self = _self;
            printf("Reverb: tick=%d channel=%d value=%d\n",
                    self->tick, self->channel, self->value);
        }
        break;
    }
}
