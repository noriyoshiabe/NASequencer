#include "Sequence.h"
#include <NACFHelper.h>
#include <string.h>

void SequenceAddEvents(Sequence *self, CFArrayRef events)
{
    CFArrayAppendArray(self->events, events, CFRangeMake(0, CFArrayGetCount(events)));
}

static void *__SequenceInit(void *_self, ...)
{
    Sequence *self = _self;
    self->events = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    return self;
}

static void __SequenceDestroy(void *_self)
{
    Sequence *self = _self;
    CFRelease(self->events);
    if (self->title) {
        free(self->title);
    }
}

static void *__SequenceDescription(const void *_self)
{
    const Sequence *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<Sequence: resolution=%d title=%s events=%@>"), self->resolution, self->title, self->events);
}

NADeclareVtbl(Sequence, NAType,
        __SequenceInit,
        __SequenceDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        __SequenceDescription,
        );

NADeclareClass(Sequence, NAType);


void TimeTableAddTimeEvent(TimeTable *self, TimeEvent *timeEvent)
{
    CFArrayAppendValue(self->timeEvents, timeEvent);
}

static void *__TimeTableInit(void *_self, ...)
{
    TimeTable *self = _self;
    self->timeEvents = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    return self;
}

static void __TimeTableDestroy(void *_self)
{
    TimeTable *self = _self;
    CFRelease(self->timeEvents);
}

static void *__TimeTableDescription(const void *_self)
{
    const TimeTable *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<TimeTable: timeEvents=%@>"), self->timeEvents);
}

NADeclareVtbl(TimeTable, NAType,
        __TimeTableInit,
        __TimeTableDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        __TimeTableDescription,
        );

NADeclareClass(TimeTable, NAType);


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

NADeclareVtbl(Pattern, NAType,
        __PatternInit,
        __PatternDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        __PatternDescription,
        );

NADeclareClass(Pattern, NAType);


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

NADeclareVtbl(Track, NAType,
        __TrackInit,
        __TrackDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        __TrackDescription
        );

NADeclareClass(Track, NAType);


static int __MidiEventCompare(const void *self, const void *to)
{
    return ((MidiEvent *)self)->tick - ((MidiEvent *)to)->tick;
}

NADeclareVtbl(MidiEvent, NAType, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
NADeclareClass(MidiEvent, NAType);


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

NADeclareVtbl(TimeEvent, NAType,
        __TimeEventInit,
        NULL,
        NULL,
        NULL,
        __MidiEventCompare,
        NULL,
        __TimeEventDescription
        );

NADeclareClass(TimeEvent, NAType);


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

NADeclareVtbl(TempoEvent, NAType,
        __TempoEventInit,
        NULL,
        NULL,
        NULL,
        __MidiEventCompare,
        NULL,
        __TempoEventDescription
        );

NADeclareClass(TempoEvent, NAType);


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

NADeclareVtbl(MarkerEvent, NAType,
        __MarkerEventInit,
        __MarkerEventDestroy,
        NULL,
        NULL,
        __MidiEventCompare,
        NULL,
        __MarkerEventDescription
        );

NADeclareClass(MarkerEvent, NAType);


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

NADeclareVtbl(SoundSelectEvent, NAType,
        __SoundSelectEventInit,
        NULL,
        NULL,
        NULL,
        __MidiEventCompare,
        NULL,
        __SoundSelectEventDescription
        );

NADeclareClass(SoundSelectEvent, NAType);


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
        __NoteEventDescription,
        );

NADeclareClass(NoteEvent, NAType);
