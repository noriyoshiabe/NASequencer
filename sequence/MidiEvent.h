#pragma once

#include "NoteTable.h"

typedef enum {
    MidiEventTypeNote,
    MidiEventTypeTempo,
    MidiEventTypeTime,
    MidiEventTypeKey,
    MidiEventTypeMarker,
    MidiEventTypeVoice,
    MidiEventTypeVolume,
    MidiEventTypePan,
    MidiEventTypeChorus,
    MidiEventTypeReverb,
} MidiEventType;

typedef struct _MidiEvent {
    MidiEventType type;
    int tick;
} MidiEvent;

typedef struct _NoteEvent {
    MidiEventType type;
    int tick;
    int channel;
    int noteNo;
    int gatetime;
    int velocity;
} NoteEvent;

typedef struct _TempoEvent {
    MidiEventType type;
    int tick;
    float tempo;
} TempoEvent;

typedef struct _TimeEvent {
    MidiEventType type;
    int tick;
    int numerator;
    int denominator;
} TimeEvent;

typedef struct _KeyEvent {
    MidiEventType type;
    int tick;
    KeySign keySign;
} KeyEvent;

typedef struct _MarkerEvent {
    MidiEventType type;
    int tick;
    int length;
    char text[];
} MarkerEvent;

typedef struct _VoiceEvent {
    MidiEventType type;
    int tick;
    int channel;
    int msb;
    int lsb;
    int programNo;
} VoiceEvent;

typedef struct _VolumeEvent {
    MidiEventType type;
    int tick;
    int channel;
    int value;
} VolumeEvent;

typedef struct _PanEvent {
    MidiEventType type;
    int tick;
    int channel;
    int value;
} PanEvent;

typedef struct _ChorusEvent {
    MidiEventType type;
    int tick;
    int channel;
    int value;
} ChorusEvent;

typedef struct _ReverbEvent {
    MidiEventType type;
    int tick;
    int channel;
    int value;
} ReverbEvent;

extern void *MidiEventAlloc(MidiEventType type, int tick, int extraSize);
