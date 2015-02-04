#pragma once

#include <NAType.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct _TimeTable {
    NAType _;
    uint16_t resolution;
    CFMutableArrayRef timeEvents;
    CFMutableArrayRef tempoEvents;
} TimeTable;

typedef struct _Sequence {
    NAType _;
    uint16_t resolution;
    char *title;
    TimeTable *timeTable;
    CFMutableArrayRef events;
} Sequence;

typedef struct _Pattern {
    NAType _;
    TimeTable *timeTable;
    CFMutableArrayRef events;
    uint32_t length;
} Pattern;

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
NAExportClass(TimeTable);
NAExportClass(Pattern);
NAExportClass(MidiEvent);
NAExportClass(TimeEvent);
NAExportClass(TempoEvent);
NAExportClass(MarkerEvent);
NAExportClass(SoundSelectEvent);
NAExportClass(NoteEvent);

extern void SequenceSetTimeTable(Sequence *self, TimeTable *timeTable);
extern void SequenceAddEvents(Sequence *self, CFArrayRef events);
extern void TimeTableAddTimeEvent(TimeTable *self, TimeEvent *timeEvent);
extern void TimeTableAddTempoEvent(TimeTable *self, TempoEvent *tempoEvent);
extern uint32_t TimeTableLocation2Tick(TimeTable *self, int32_t measure, int32_t beat, int32_t tick);
extern uint32_t TimeTableMBLength2Tick(TimeTable *self, int32_t offsetTick, int32_t measure, int32_t beat);
