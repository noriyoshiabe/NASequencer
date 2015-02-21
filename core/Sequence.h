#pragma once

#include <NAType.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct _Location {
    int32_t m;
    int16_t b;
    int16_t t;
} Location;

typedef struct _TimeTable {
    NAType _;
    int16_t resolution;
    CFMutableArrayRef timeEvents;
    CFMutableArrayRef tempoEvents;
} TimeTable;

typedef struct _Sequence {
    NAType _;
    int16_t resolution;
    CFStringRef title;
    TimeTable *timeTable;
    CFMutableArrayRef events;
    int32_t length;
} Sequence;

typedef struct _Pattern {
    NAType _;
    CFStringRef name;
    TimeTable *timeTable;
    CFMutableArrayRef events;
    int32_t length;
} Pattern;

typedef struct _MidiEvent {
    NAType _;
    int32_t tick;
} MidiEvent;

typedef struct _TimeEvent {
    MidiEvent _;
    int16_t numerator;
    int16_t denominator;
} TimeEvent;

typedef struct _TempoEvent {
    MidiEvent _;
    float tempo;
} TempoEvent;

typedef struct _MarkerEvent {
    MidiEvent _;
    CFStringRef text;
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
extern int32_t TimeTableLocation2Tick(TimeTable *self, int32_t measure, int16_t beat, int16_t tick);
extern int32_t TimeTableMBLength2Tick(TimeTable *self, int32_t offsetTick, int32_t measure, int16_t beat);
extern Location TimeTableTick2Location(TimeTable *self, int32_t tick);
extern int32_t TimeTableMicroSec2Tick(TimeTable *self, int64_t usec);
extern Location TimeTableMicroSec2Location(TimeTable *self, int64_t usec);
extern int64_t TimeTableTick2MicroSec(TimeTable *self, int32_t tick);
extern void TimeTableGetTempoByTick(TimeTable *self, int32_t tick, float *tempo);
extern void TimeTableGetTimeSignByTick(TimeTable *self, int32_t tick, int16_t *numerator, int16_t *denominator);


typedef struct _SequenceVisitorVtbl {
    void (*visitSequence)(void *self, Sequence *elem);
    void (*visitPattern)(void *self, Pattern *elem);
    void (*visitTimeTable)(void *self, TimeTable *elem);
    void (*visitTimeEvent)(void *self, TimeEvent *elem);
    void (*visitTempoEvent)(void *self, TempoEvent *elem);
    void (*visitMarkerEvent)(void *self, MarkerEvent *elem);
    void (*visitSoundSelectEvent)(void *self, SoundSelectEvent *elem);
    void (*visitNoteEvent)(void *self, NoteEvent *elem);
} SequenceVisitorVtbl;

NAExportClass(SequenceVisitor);

typedef struct _SequenceElementVtbl {
    void (*accept)(void *self, void *visitor);
} SequenceElementVtbl;

NAExportClass(SequenceElement);

extern void SequenceElementAccept(void *self, void *visitor);
