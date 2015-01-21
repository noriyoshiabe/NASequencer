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

typedef struct _Pattern {
    NAType _;
    CFMutableArrayRef tracks;
} Pattern;
extern NAClass PatternClass;
extern char PatternID[];

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

typedef struct _TimeEvent {
    MidiEvent _;
    uint8_t numerator;
    uint8_t denominator;
} TimeEvent;
extern NAClass TimeEventClass;
extern char TimeEventID[];

typedef struct _TempoEvent {
    MidiEvent _;
    float tempo;
} TempoEvent;
extern char TempoEventID[];

typedef struct _MarkerEvent {
    MidiEvent _;
    char *text;
} MarkerEvent;
extern NAClass MarkerEventClass;
extern char MarkerEventID[];

typedef struct _SoundSelectEvent {
    MidiEvent _;
    uint8_t channel;
    uint8_t msb;
    uint8_t lsb;
    uint8_t programNo;
} SoundSelectEvent;
extern NAClass SoundSelectEventClass;
extern char SoundSelectEventID[];

typedef struct _NoteEvent {
    MidiEvent _;
    uint8_t channel;
    uint8_t noteNo;
    uint8_t velocity;
    uint32_t gatetime;
} NoteEvent;
extern NAClass NoteEventClass;
extern char NoteEventID[];

