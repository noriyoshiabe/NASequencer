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

static void *__SequenceDescription(const void *_self)
{
    const Sequence *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<Sequence: resolution=%d title=%s tracks=%@>"), self->resolution, self->title, self->tracks);
}

static NATypeVtbl __Sequence__typeVtbl = {
    __SequenceInit,
    __SequenceDestroy,
    NULL,
    NULL,
    NULL,
    NULL,
    __SequenceDescription,
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


static void *__PatternInit(void *_self, ...)
{
    Pattern *self = _self;
    self->tracks = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    return self;
}

static void __PatternDestroy(void *_self)
{
    Pattern *self = _self;
    CFRelease(self->tracks);
}

static void *__PatternDescription(const void *_self)
{
    const Pattern *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<Pattern: tracks=%@>"), self->tracks);
}

static NATypeVtbl __Pattern__typeVtbl = {
    __PatternInit,
    __PatternDestroy,
    NULL,
    NULL,
    NULL,
    NULL,
    __PatternDescription,
};

static NAVtblEntry __Pattern__vEntries[] = {
    {NATypeID, &__Pattern__typeVtbl},
    {NULL, NULL},
};

char PatternID[] = "Pattern";
NAClass PatternClass = {
    PatternID,
    sizeof(Pattern),
    __Pattern__vEntries,
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

static void *__TrackDescription(const void *_self)
{
    const Track *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<Track: events=%@>"), self->events);
}

static NATypeVtbl __Track__typeVtbl = {
    __TrackInit,
    __TrackDestroy,
    NULL,
    NULL,
    NULL,
    NULL,
    __TrackDescription
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
