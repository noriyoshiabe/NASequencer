#include "Sequence.h"
#include <NACFHelper.h>
#include <string.h>

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


static int __MidiEventCompare(const void *self, const void *to)
{
    return ((MidiEvent *)self)->tick - ((MidiEvent *)to)->tick;
}

char MidiEventID[] = "MidiEvent";


static void *__TimeEventInit(void *_self, ...)
{
    TimeEvent *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->_.tick = va_arg(ap, uint32_t);
    self->numerator = (uint8_t)va_arg(ap, uint32_t);
    self->denominator = (uint8_t)va_arg(ap, uint32_t);
    va_end(ap);

    return self;
}

static void *__TimeEventDescription(const void *_self)
{
    const TimeEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<TimeEvent: tick=%d numerator=%d denominator=%d>"), self->_.tick, self->numerator, self->denominator);
}

static NATypeVtbl __TimeEvent__typeVtbl = {
    __TimeEventInit,
    NULL,
    NULL,
    NULL,
    __MidiEventCompare,
    NULL,
    __TimeEventDescription
};

static NAVtblEntry __TimeEvent__vEntries[] = {
    {NATypeID, &__TimeEvent__typeVtbl},
    {NULL, NULL},
};

char TimeEventID[] = "TimeEvent";
NAClass TimeEventClass = {
    TimeEventID,
    sizeof(TimeEvent),
    __TimeEvent__vEntries,
};


static void *__TempoEventInit(void *_self, ...)
{
    TempoEvent *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->_.tick = va_arg(ap, uint32_t);
    self->tempo = (float)va_arg(ap, double);
    va_end(ap);

    return self;
}

static void *__TempoEventDescription(const void *_self)
{
    const TempoEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<TempoEvent: tick=%d tempo=%f>"), self->_.tick, self->tempo);
}

static NATypeVtbl __TempoEvent__typeVtbl = {
    __TempoEventInit,
    NULL,
    NULL,
    NULL,
    __MidiEventCompare,
    NULL,
    __TempoEventDescription
};

static NAVtblEntry __TempoEvent__vEntries[] = {
    {NATypeID, &__TempoEvent__typeVtbl},
    {NULL, NULL},
};

char TempoEventID[] = "TempoEvent";
NAClass TempoEventClass = {
    TempoEventID,
    sizeof(TempoEvent),
    __TempoEvent__vEntries,
};


static void *__MarkerEventInit(void *_self, ...)
{
    MarkerEvent *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->_.tick = va_arg(ap, uint32_t);
    const char *text = va_arg(ap, const char *);
    va_end(ap);

    self->text = malloc(strlen(text) + 1);
    strcpy(self->text, text);

    return self;
}

static void __MarkerEventDestroy(void *_self)
{
    MarkerEvent *self = _self;
    free(self->text);
}

static void *__MarkerEventDescription(const void *_self)
{
    const MarkerEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<MarkerEvent: tick=%d text=%s>"), self->_.tick, self->text);
}

static NATypeVtbl __MarkerEvent__typeVtbl = {
    __MarkerEventInit,
    __MarkerEventDestroy,
    NULL,
    NULL,
    __MidiEventCompare,
    NULL,
    __MarkerEventDescription
};

static NAVtblEntry __MarkerEvent__vEntries[] = {
    {NATypeID, &__MarkerEvent__typeVtbl},
    {NULL, NULL},
};

char MarkerEventID[] = "MarkerEvent";
NAClass MarkerEventClass = {
    MarkerEventID,
    sizeof(MarkerEvent),
    __MarkerEvent__vEntries,
};


static void *__SoundSelectEventInit(void *_self, ...)
{
    SoundSelectEvent *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->_.tick = va_arg(ap, uint32_t);
    self->channel = (uint8_t)va_arg(ap, uint32_t);
    self->msb = (uint8_t)va_arg(ap, uint32_t);
    self->lsb = (uint8_t)va_arg(ap, uint32_t);
    self->programNo = (uint8_t)va_arg(ap, uint32_t);
    va_end(ap);

    return self;
}

static void *__SoundSelectEventDescription(const void *_self)
{
    const SoundSelectEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<SoundSelectEvent: tick=%d channel=%d msb=%d lsb=%d programNo=%d>"), self->_.tick, self->channel, self->msb, self->lsb, self->programNo);
}

static NATypeVtbl __SoundSelectEvent__typeVtbl = {
    __SoundSelectEventInit,
    NULL,
    NULL,
    NULL,
    __MidiEventCompare,
    NULL,
    __SoundSelectEventDescription
};

static NAVtblEntry __SoundSelectEvent__vEntries[] = {
    {NATypeID, &__SoundSelectEvent__typeVtbl},
    {NULL, NULL},
};

char SoundSelectEventID[] = "SoundSelectEvent";
NAClass SoundSelectEventClass = {
    SoundSelectEventID,
    sizeof(SoundSelectEvent),
    __SoundSelectEvent__vEntries,
};


static void *__NoteEventInit(void *_self, ...)
{
    NoteEvent *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->_.tick = va_arg(ap, uint32_t);
    self->channel = (uint8_t)va_arg(ap, uint32_t);
    self->noteNo = (uint8_t)va_arg(ap, uint32_t);
    self->velocity = (uint8_t)va_arg(ap, uint32_t);
    self->gatetime = va_arg(ap, uint32_t);
    va_end(ap);

    return self;
}

static void *__NoteEventDescription(const void *_self)
{
    const NoteEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<NoteEvent: tick=%d channel=%d noteNo=%d velocity=%d gatetime=%d>"), self->_.tick, self->channel, self->noteNo, self->velocity, self->gatetime);
}

NADeclareVtbl(NoteEvent, NAType,
    __NoteEventInit,
    NULL,
    NULL,
    NULL,
    __MidiEventCompare,
    NULL,
    __NoteEventDescription
);

NADeclareVtblEntry(NoteEvent, NAType);
NADeclareClass(NoteEvent);
