#pragma once

#include "MidiEvent.h"
#include "TimeTable.h"
#include "NAArray.h"
#include "NADescription.h"

typedef struct _Sequence Sequence;
struct _Sequence {
    char *title;
    TimeTable *timeTable;
    NAArray *events;
    NAArray *children;
};

extern Sequence *SequenceCreate();
extern void SequenceRelease(Sequence *self);
extern Sequence *SequenceRetain(Sequence *self);
extern void SequenceSortEvents(Sequence *self);
extern void SequenceDump(Sequence *self, int indent);
