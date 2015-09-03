#include "MidiEvent.h"

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

void MidiEventDescription(void *_self, FILE *stream)
{
    switch (((MidiEvent *)_self)->type) {
    case MidiEventTypeNote:
        {
            NoteEvent *self = _self;
            fprintf(stream, "Note: tick=%d channel=%d noteNo=%d gatetime=%d velocity=%d",
                    self->tick, self->channel, self->noteNo, self->gatetime, self->velocity);
        }
        break;
    case MidiEventTypeTempo:
        {
            TempoEvent *self = _self;
            fprintf(stream, "Tempo: tick=%d tempo=%f",
                    self->tick, self->tempo);
        }
        break;
    case MidiEventTypeTime:
        {
            TimeEvent *self = _self;
            fprintf(stream, "Time: tick=%d time=%d/%d",
                    self->tick, self->numerator, self->denominator);
        }
        break;
    case MidiEventTypeKey:
        {
            KeyEvent *self = _self;
            fprintf(stream, "Key: tick=%d key=%s",
                    self->tick, KeySign2String(self->keySign));
        }
        break;
    case MidiEventTypeMarker:
        {
            MarkerEvent *self = _self;
            fprintf(stream, "Marker: tick=%d text=%s",
                    self->tick, self->text);
        }
        break;
    case MidiEventTypeVoice:
        {
            VoiceEvent *self = _self;
            fprintf(stream, "Voice: tick=%d channel=%d msb=%d lsb=%d programNo=%d",
                    self->tick, self->channel, self->msb, self->lsb, self->programNo);
        }
        break;
    case MidiEventTypeVolume:
        {
            VolumeEvent *self = _self;
            fprintf(stream, "Volume: tick=%d channel=%d value=%d",
                    self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypePan:
        {
            PanEvent *self = _self;
            fprintf(stream, "Pan: tick=%d channel=%d value=%d",
                    self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeChorus:
        {
            ChorusEvent *self = _self;
            fprintf(stream, "Chorus: tick=%d channel=%d value=%d",
                    self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeReverb:
        {
            ReverbEvent *self = _self;
            fprintf(stream, "Reverb: tick=%d channel=%d value=%d",
                    self->tick, self->channel, self->value);
        }
        break;
    }
}
