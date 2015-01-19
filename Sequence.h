#pragma once

#include <NAType.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct _Sequence {
    NAType _;
    uint16_t resolution;
    char *title;
    CFMutableArrayRef tracks;
} Sequence;
extern NAClass SequenceClass;
extern char SequenceID[];

typedef struct _Track {
    NAType _;
    CFMutableArrayRef events;
} Track;
extern NAClass TrackClass;
extern char TrackID[];

typedef struct _MidiEvent {
    NAType _;
    uint32_t tick;
} MidiEvent;
extern char MidiEventID[];
