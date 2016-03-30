#pragma once

#include <stdint.h>

typedef enum {
    MidiEventTypeNote,
    MidiEventTypeTempo,
    MidiEventTypeTime,
    MidiEventTypeKey,
    MidiEventTypeTitle,
    MidiEventTypeCopyright,
    MidiEventTypeMarker,
    MidiEventTypeBank,
    MidiEventTypeProgram,
    MidiEventTypeVolume,
    MidiEventTypePan,
    MidiEventTypeChorus,
    MidiEventTypeReverb,
    MidiEventTypeExpression,
    MidiEventTypePitch,

    MidiEventTypeDetune,
    MidiEventTypePitchSense,
    MidiEventTypeSynth,
} MidiEventType;

typedef struct _MidiEvent {
    MidiEventType type;
    int id;
    int tick;
} MidiEvent;

typedef struct _ChannelEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
} ChannelEvent;

typedef struct _NoteEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int noteNo;
    int gatetime;
    int velocity;
} NoteEvent;

typedef struct _TempoEvent {
    MidiEventType type;
    int id;
    int tick;
    float tempo;
} TempoEvent;

typedef struct _TimeEvent {
    MidiEventType type;
    int id;
    int tick;
    int numerator;
    int denominator;
} TimeEvent;

typedef struct _KeyEvent {
    MidiEventType type;
    int id;
    int tick;
    int sf;
    int mi;
} KeyEvent;

typedef struct _TitleEvent {
    MidiEventType type;
    int id;
    int tick;
    char text[];
} TitleEvent;

typedef struct _MarkerEvent {
    MidiEventType type;
    int id;
    int tick;
    char text[];
} MarkerEvent;

typedef struct _CopyrightEvent {
    MidiEventType type;
    int id;
    int tick;
    char text[];
} CopyrightEvent;

typedef struct _BankEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int bankNo;
} BankEvent;

typedef struct _ProgramEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int programNo;
} ProgramEvent;

typedef struct _VolumeEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int value;
} VolumeEvent;

typedef struct _PanEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int value;
} PanEvent;

typedef struct _ChorusEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int value;
} ChorusEvent;

typedef struct _ReverbEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int value;
} ReverbEvent;

typedef struct _ExpressionEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int value;
} ExpressionEvent;

typedef struct _PitchEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int value;
} PitchEvent;

typedef struct _DetuneEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int value;
    struct {
        uint8_t msb;
        uint8_t lsb;
    } fine;
    struct {
        uint8_t msb;
    } corse;
} DetuneEvent;

typedef struct _PitchSenseEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int value;
} PitchSenseEvent;

typedef struct _SynthEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    char identifier[];
} SynthEvent;

extern void *MidiEventAlloc(MidiEventType type, int id, int tick, int extraSize);
extern void MidiEventDump(MidiEvent *self, int indent);
extern int MidiEventGetChannel(const MidiEvent *event);
extern int MidiEventComparator(const void *event1, const void *event2);
extern int MidiEventIDComparator(const void *event1, const void *event2);

static inline char *MidiEventType2String(MidiEventType type)
{
#define CASE(type) case type: return &(#type[13])
    switch (type) {
    CASE(MidiEventTypeNote);
    CASE(MidiEventTypeTempo);
    CASE(MidiEventTypeTime);
    CASE(MidiEventTypeKey);
    CASE(MidiEventTypeTitle);
    CASE(MidiEventTypeCopyright);
    CASE(MidiEventTypeMarker);
    CASE(MidiEventTypeBank);
    CASE(MidiEventTypeProgram);
    CASE(MidiEventTypeVolume);
    CASE(MidiEventTypePan);
    CASE(MidiEventTypeChorus);
    CASE(MidiEventTypeReverb);
    CASE(MidiEventTypeExpression);
    CASE(MidiEventTypePitch);
    CASE(MidiEventTypeDetune);
    CASE(MidiEventTypePitchSense);
    CASE(MidiEventTypeSynth);

    default:
       break;
    }

    return "Unknown event type";
#undef CASE
}
