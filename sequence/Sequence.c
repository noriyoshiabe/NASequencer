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
    self->sequence.events = NAArrayCreate(32, NULL);
    self->sequence.children = NAArrayCreate(8, NULL);
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

        NAArrayTraverse(self->events, free);
        NAArrayDestroy(self->events);

        NAArrayTraverse(self->children, (void *)SequenceRelease);
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
