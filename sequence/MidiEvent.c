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
