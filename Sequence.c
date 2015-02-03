#include "Sequence.h"
#include <NACFHelper.h>
#include <string.h>

void SequenceSetTimeTable(Sequence *self, TimeTable *timeTable)
{
    self->timeTable = NARetain(timeTable);
}

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

    if (self->title) {
        free(self->title);
    }

    if (self->timeTable) {
        NARelease(self->timeTable);
    }

    CFRelease(self->events);
}

static void *__SequenceDescription(const void *_self)
{
    const Sequence *self = _self;

    CFStringRef cfString = NADescription(self->timeTable);
    void *ret = (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<Sequence: resolution=%d title=%s timeTable=%@ events=%@>"), self->resolution, self->title, cfString, self->events);
    CFRelease(cfString);

    return ret;
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
    CFIndex i;
    CFIndex count = CFArrayGetCount(self->timeEvents);
    for (i = 0; i < count; ++i) {
        TimeEvent *event = (TimeEvent *)CFArrayGetValueAtIndex(self->timeEvents, i);
        int compare = timeEvent->_.tick - event->_.tick;
        if (0 == compare) {
            event->numerator = timeEvent->numerator;
            event->denominator = timeEvent->denominator;
            return;
        }
        else if (compare < 0) {
            break;
        }
    }

    CFArrayInsertValueAtIndex(self->timeEvents, i, timeEvent);
}

void TimeTableAddTempoEvent(TimeTable *self, TempoEvent *tempoEvent)
{
    CFIndex i;
    CFIndex count = CFArrayGetCount(self->tempoEvents);
    for (i = 0; i < count; ++i) {
        TempoEvent *event = (TempoEvent *)CFArrayGetValueAtIndex(self->tempoEvents, i);
        int compare = tempoEvent->_.tick - event->_.tick;
        if (0 == compare) {
            event->tempo = tempoEvent->tempo;
            return;
        }
        else if (compare < 0) {
            break;
        }
    }

    CFArrayInsertValueAtIndex(self->tempoEvents, i, tempoEvent);
}

uint32_t TimeTableLocation2Tick(TimeTable *self, int32_t measure, int32_t beat, int32_t tick)
{
    int32_t offsetTick = 0;
    
    int32_t tickPerBeat = self->resolution * 4 / 4;
    int32_t tickPerMeasure = tickPerBeat * 4;

    measure -= 1;
    beat -= 1;

    CFIndex count = CFArrayGetCount(self->timeEvents);
    for (int i = 0; i < count; ++i) {
        const TimeEvent *event = CFArrayGetValueAtIndex(self->timeEvents, i);

        int32_t _tick = tickPerMeasure * measure + tickPerBeat * beat + tick + offsetTick;
        if (event->_.tick <= _tick) {
            offsetTick = event->_.tick;

            tickPerBeat = self->resolution * 4 / event->denominator;
            tickPerMeasure = tickPerBeat * event->numerator;

            tick -= event->_.tick % tickPerBeat;
            measure -= event->_.tick / tickPerMeasure;
            beat -= (event->_.tick % tickPerMeasure) / tickPerBeat;
        }
        else {
            break;
        }
    }

    return tickPerMeasure * measure + tickPerBeat * beat + offsetTick;
}

static void *__TimeTableInit(void *_self, ...)
{
    TimeTable *self = _self;
    self->timeEvents = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    self->tempoEvents = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    return self;
}

static void *__TimeTableCopy(const void *_self)
{
    const TimeTable *self = _self;
    TimeTable *copied = NATypeNew(TimeTable);

    copied->resolution = self->resolution;
    CFArrayAppendArray(copied->timeEvents, self->timeEvents, CFRangeMake(0, CFArrayGetCount(self->timeEvents)));
    CFArrayAppendArray(copied->tempoEvents, self->tempoEvents, CFRangeMake(0, CFArrayGetCount(self->tempoEvents)));

    return copied;
}

static void __TimeTableDestroy(void *_self)
{
    TimeTable *self = _self;
    CFRelease(self->timeEvents);
    CFRelease(self->tempoEvents);
}

static void *__TimeTableDescription(const void *_self)
{
    const TimeTable *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<TimeTable: timeEvents=%@ tempoEvents=%@>"), self->timeEvents, self->tempoEvents);
}

NADeclareVtbl(TimeTable, NAType,
        __TimeTableInit,
        __TimeTableDestroy,
        NULL,
        NULL,
        NULL,
        __TimeTableCopy,
        __TimeTableDescription,
        );

NADeclareClass(TimeTable, NAType);


static void *__PatternInit(void *_self, ...)
{
    Pattern *self = _self;

    va_list ap;
    va_start(ap, _self);
    self->timeTable = NARetain(va_arg(ap, TimeTable *));
    self->events = (CFMutableArrayRef)CFRetain(va_arg(ap, CFMutableArrayRef));
    self->length = va_arg(ap, uint32_t);
    va_end(ap);
    
    return self;
}

static void *__PatternCopy(const void *_self)
{
    const Pattern *self = _self;

    TimeTable *copiedTimeTable = NACopy(self->timeTable);
    CFMutableArrayRef copiedEvents = CFArrayCreateMutableCopy(NULL, 0, self->events);
        
    Pattern *copied = NATypeNew(Pattern, copiedTimeTable, copiedEvents);

    NARelease(copiedTimeTable);
    CFRelease(copiedEvents);

    return copied;
}

static void __PatternDestroy(void *_self)
{
    Pattern *self = _self;
    NARelease(self->timeTable);
    CFRelease(self->events);
}

static void *__PatternDescription(const void *_self)
{
    const Pattern *self = _self;

    CFStringRef cfString = NADescription(self->timeTable);
    void *ret = (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<Pattern: timeTable=%@ events=%@>"), cfString, self->events);
    CFRelease(cfString);

    return ret;
}

NADeclareVtbl(Pattern, NAType,
        __PatternInit,
        __PatternDestroy,
        NULL,
        NULL,
        NULL,
        __PatternCopy,
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


static void *__MidiEventInit(void *_self, ...)
{
    MidiEvent *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->tick = va_arg(ap, uint32_t);
    va_end(ap);

    return self;
}

static int __MidiEventCompare(const void *self, const void *to)
{
    return ((MidiEvent *)self)->tick - ((MidiEvent *)to)->tick;
}

void *__MidiEventCopy(const void *_self)
{
    const MidiEvent *self = _self;

    MidiEvent *copied = (MidiEvent *)__MidiEventInit(NATypeAlloc(self->_.clazz), self->tick);
    memcpy(&copied->tick, &self->tick, self->_.clazz->size - sizeof(NAType));
    return copied;
}

NADeclareVtbl(MidiEvent, NAType, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
NADeclareClass(MidiEvent, NAType);


static void *__TimeEventDescription(const void *_self)
{
    const TimeEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<TimeEvent: tick=%d numerator=%d denominator=%d>"), self->_.tick, self->numerator, self->denominator);
}

NADeclareVtbl(TimeEvent, NAType,
        __MidiEventInit,
        NULL,
        NULL,
        NULL,
        __MidiEventCompare,
        __MidiEventCopy,
        __TimeEventDescription
        );

NADeclareClass(TimeEvent, NAType);


static void *__TempoEventDescription(const void *_self)
{
    const TempoEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<TempoEvent: tick=%d tempo=%f>"), self->_.tick, self->tempo);
}

NADeclareVtbl(TempoEvent, NAType,
        __MidiEventInit,
        NULL,
        NULL,
        NULL,
        __MidiEventCompare,
        __MidiEventCopy,
        __TempoEventDescription
        );

NADeclareClass(TempoEvent, NAType);


static void __MarkerEventDestroy(void *_self)
{
    MarkerEvent *self = _self;
    free(self->text);
}

void *__MarkerEventCopy(const void *_self)
{
    const MarkerEvent *self = _self;

    MarkerEvent *copied = (MarkerEvent *)__MidiEventInit(NATypeAlloc(self->_._.clazz), self->_.tick);
    copied->text = strdup(self->text);
    return copied;
}

static void *__MarkerEventDescription(const void *_self)
{
    const MarkerEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<MarkerEvent: tick=%d text=%s>"), self->_.tick, self->text);
}

NADeclareVtbl(MarkerEvent, NAType,
        __MidiEventInit,
        __MarkerEventDestroy,
        NULL,
        NULL,
        __MidiEventCompare,
        __MarkerEventCopy,
        __MarkerEventDescription
        );

NADeclareClass(MarkerEvent, NAType);


static void *__SoundSelectEventDescription(const void *_self)
{
    const SoundSelectEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<SoundSelectEvent: tick=%d channel=%d msb=%d lsb=%d programNo=%d>"), self->_.tick, self->channel, self->msb, self->lsb, self->programNo);
}

NADeclareVtbl(SoundSelectEvent, NAType,
        __MidiEventInit,
        NULL,
        NULL,
        NULL,
        __MidiEventCompare,
        __MidiEventCopy,
        __SoundSelectEventDescription
        );

NADeclareClass(SoundSelectEvent, NAType);


static void *__NoteEventDescription(const void *_self)
{
    const NoteEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<NoteEvent: tick=%d channel=%d noteNo=%d velocity=%d gatetime=%d>"), self->_.tick, self->channel, self->noteNo, self->velocity, self->gatetime);
}

NADeclareVtbl(NoteEvent, NAType,
        __MidiEventInit,
        NULL,
        NULL,
        NULL,
        __MidiEventCompare,
        __MidiEventCopy,
        __NoteEventDescription,
        );

NADeclareClass(NoteEvent, NAType);
