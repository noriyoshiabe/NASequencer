#pragma once

#include "MidiEvent.h"
#include "TimeTable.h"

typedef struct _Sequence Sequence;
struct _Sequence {
    const char *title;
    const int eventCount;
    const TimeTable *timeTable;
    const MidiEvent *events;
    const int childCount;
    const Sequence **children;
};

extern Sequence *SequenceCreate();
extern void SequenceRelease(Sequence *self);
extern void SequenceAppendChild(Sequence *self, Sequence *child);
extern void SequenceAppendEvent(Sequence *self, MidiEvent *event);
extern void SequenceSortEvents(Sequence *self);
