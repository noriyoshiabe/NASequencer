#pragma once

#include <NAType.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct _Sequence {
    NAType _;
    uint16_t resolution;
    char *title;
    CFMutableArrayRef tracks;
} Sequence;

typedef struct _Pattern {
    NAType _;
    CFMutableArrayRef tracks;
} Pattern;

typedef struct _Track {
    NAType _;
    CFMutableArrayRef events;
} Track;

typedef struct _MidiEvent {
    NAType _;
    uint32_t tick;
} MidiEvent;

typedef struct _TimeEvent {
    MidiEvent _;
    uint8_t numerator;
    uint8_t denominator;
} TimeEvent;

typedef struct _TempoEvent {
    MidiEvent _;
    float tempo;
} TempoEvent;

typedef struct _MarkerEvent {
    MidiEvent _;
    char *text;
} MarkerEvent;

typedef struct _SoundSelectEvent {
    MidiEvent _;
    uint8_t channel;
    uint8_t msb;
    uint8_t lsb;
    uint8_t programNo;
} SoundSelectEvent;

typedef struct _NoteEvent {
    MidiEvent _;
    uint8_t channel;
    uint8_t noteNo;
    uint8_t velocity;
    uint32_t gatetime;
} NoteEvent;

NAExportClass(Sequence);
NAExportClass(Pattern);
NAExportClass(Track);
NAExportClass(MidiEvent);
NAExportClass(TimeEvent);
NAExportClass(TempoEvent);
NAExportClass(MarkerEvent);
NAExportClass(SouneSelectEvent);
NAExportClass(NoteEvent);
