#include "Sequence.h"
#include <NACFHelper.h>

static void *__SequenceInit(void *_self, ...)
{
    Sequence *self = _self;
    self->tracks = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    return self;
}

static void __SequenceDestroy(void *_self)
{
    Sequence *self = _self;
    CFRelease(self->tracks);
    if (self->title) {
        free(self->title);
    }
}

static NATypeVtbl __Sequence__typeVtbl = {
    __SequenceInit,
    __SequenceDestroy,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, // TODO
};

static NAVtblEntry __Sequence__vEntries[] = {
    {NATypeID, &__Sequence__typeVtbl},
    {NULL, NULL},
};

char SequenceID[] = "Sequence";
NAClass SequenceClass = {
    SequenceID,
    sizeof(Sequence),
    __Sequence__vEntries,
};


static void *__TrackInit(void *_self, ...)
{
    Track *self = _self;
    self->events = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    return self;
}

static void __TrackDestroy(void *_self)
{
    Track *self = _self;
    CFRelease(self->events);
}

static NATypeVtbl __Track__typeVtbl = {
    __TrackInit,
    __TrackDestroy,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, // TODO
};

static NAVtblEntry __Track__vEntries[] = {
    {NATypeID, &__Track__typeVtbl},
    {NULL, NULL},
};

char TrackID[] = "Track";
NAClass TrackClass = {
    TrackID,
    sizeof(Track),
    __Track__vEntries,
};


char MidiEventID[] = "MidiEvent";
