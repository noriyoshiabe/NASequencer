#include "Sequence.h"

#include <stdlib.h>

typedef struct _SequenceImpl {
    Sequence sequence;
    int rerCount;
} SequenceImpl;

Sequence *SequenceCreate()
{
    SequenceImpl *self = calloc(1, sizeof(SequenceImpl));
    self->sequence.timeTable = TimeTableCreate();
    self->sequence.events = NAArrayCreate(32, sizeof(MidiEvent *));
    self->sequence.children = NAArrayCreate(8, sizeof(Sequence *));
    self->rerCount = 1;

    return (Sequence *)self;
}

void SequenceRelease(Sequence *self)
{
    SequenceImpl *impl = (SequenceImpl *)self;

    if (0 == --impl->rerCount) {
        if (self->title) {
            free(self->title);
        }

        TimeTableDestroy(self->timeTable);

        int count = NAArrayCount(self->events);
        MidiEvent **events = NAArrayGetValues(self->events);
        for (int i = 0; i < count; ++i) {
            free(events[i]);
        }
        NAArrayDestroy(self->events);

        count = NAArrayCount(self->children);
        Sequence **children = NAArrayGetValues(self->children);
        for (int i = 0; i < count; ++i) {
            SequenceRelease(children[i]);
        }
        NAArrayDestroy(self->children);
    }
}

static int MidiEventComparator(const void *_event1, const void *_event2)
{
    const MidiEvent **event1 = (const MidiEvent **)_event1;
    const MidiEvent **event2 = (const MidiEvent **)_event2;

    return (*event1)->tick - (*event2)->tick;
}

void SequenceSortEvents(Sequence *self)
{
    NAArraySort(self->events, MidiEventComparator);
}
