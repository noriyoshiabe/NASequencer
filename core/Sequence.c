#include "Sequence.h"
#include <NACFHelper.h>
#include <string.h>
#include <math.h>

NADeclareAbstractClass(SequenceVisitor);
NADeclareAbstractClass(SequenceElement);

void SequenceSetTimeTable(Sequence *self, TimeTable *timeTable)
{
    self->timeTable = NARetain(timeTable);
}

void SequenceAddEvents(Sequence *self, CFArrayRef events)
{
    CFArrayAppendArray(self->events, events, CFRangeMake(0, CFArrayGetCount(events)));
    CFArraySortValues(self->events, CFRangeMake(0, CFArrayGetCount(self->events)), NACFComparatorFunction, NULL);
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
        CFRelease(self->title);
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
    void *ret = (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<Sequence: title=%@ timeTable=%@ events=%@>"), self->title, cfString, self->events);
    CFRelease(cfString);

    return ret;
}

static void __SequenceAccept(void *self, void *visitor)
{
    NAVtbl(visitor, SequenceVisitor)->visitSequence(visitor, self);
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
NADeclareVtbl(Sequence, SequenceElement, __SequenceAccept);
NADeclareClass(Sequence, NAType, SequenceElement);


void TimeTableAddTimeEvent(TimeTable *self, TimeEvent *timeEvent)
{
    CFIndex i;
    CFIndex count = CFArrayGetCount(self->timeEvents);
    for (i = 0; i < count; ++i) {
        const TimeEvent *event = CFArrayGetValueAtIndex(self->timeEvents, i);
        int compare = timeEvent->__.tick - event->__.tick;
        if (0 == compare) {
            CFArrayRemoveValueAtIndex(self->timeEvents, i);
            break;
        }
        else if (compare < 0) {
            break;
        }
    }

    if (0 < i) {
        const TimeEvent *prev = CFArrayGetValueAtIndex(self->timeEvents, i - 1);
        if (prev->numerator == timeEvent->numerator
                && prev->denominator == timeEvent->denominator) {
            return;
        }
    }

    CFArrayInsertValueAtIndex(self->timeEvents, i, timeEvent);
}

void TimeTableAddTempoEvent(TimeTable *self, TempoEvent *tempoEvent)
{
    CFIndex i;
    CFIndex count = CFArrayGetCount(self->tempoEvents);
    for (i = 0; i < count; ++i) {
        const TempoEvent *event = CFArrayGetValueAtIndex(self->tempoEvents, i);
        int compare = tempoEvent->__.tick - event->__.tick;
        if (0 == compare) {
            CFArrayRemoveValueAtIndex(self->tempoEvents, i);
            break;
        }
        else if (compare < 0) {
            break;
        }
    }

    if (0 < i) {
        const TempoEvent *prev = CFArrayGetValueAtIndex(self->tempoEvents, i - 1);
        if (prev->tempo == tempoEvent->tempo) {
            return;
        }
    }

    CFArrayInsertValueAtIndex(self->tempoEvents, i, tempoEvent);
}

int32_t TimeTableLocation2Tick(TimeTable *self, int32_t measure, int16_t beat, int16_t tick)
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
        if (event->__.tick <= _tick) {
            tick -= event->__.tick % tickPerBeat;
            measure -= event->__.tick / tickPerMeasure;
            beat -= (event->__.tick % tickPerMeasure) / tickPerBeat;

            tickPerBeat = self->resolution * 4 / event->denominator;
            tickPerMeasure = tickPerBeat * event->numerator;
            offsetTick = event->__.tick;
        }
        else {
            break;
        }
    }

    return tickPerMeasure * measure + tickPerBeat * beat + offsetTick;
}

int32_t TimeTableMBLength2Tick(TimeTable *self, int32_t offsetTick, int32_t measure, int16_t beat)
{
    int32_t tickPerBeat = self->resolution * 4 / 4;
    int32_t tickPerMeasure = tickPerBeat * 4;

    int32_t ret = 0;
    int32_t prev_tick = 0;

    CFIndex count = CFArrayGetCount(self->timeEvents);
    int i = 0;
    for (;;) {
        const TimeEvent *event = NULL;

        if (i < count) {
            event = CFArrayGetValueAtIndex(self->timeEvents, i);
        }

        if (0 < i) {
            for (;;) {
                if (prev_tick <= offsetTick && (!event || offsetTick < event->__.tick)) {
                    if (0 < measure) {
                        ret += tickPerMeasure;
                        offsetTick += tickPerMeasure;
                        --measure;
                    }
                    else if (0 < beat) {
                        ret += tickPerBeat;
                        offsetTick += tickPerBeat;
                        --beat;
                    }
                    else {
                        goto END;
                    }
                }
                else {
                    break;
                }
            }
        }

        if (event) {
            tickPerBeat = self->resolution * 4 / event->denominator;
            tickPerMeasure = tickPerBeat * event->numerator;

            prev_tick = event->__.tick;
        }

        ++i;
    }

END:
    return ret;
}

Location TimeTableTick2Location(TimeTable *self, int32_t tick)
{
    Location ret = {0};

    int32_t numerator = 4;
    int32_t denominator = 4;

    int32_t offsetTick = 0;
    int32_t measure = 1;

    CFIndex count = CFArrayGetCount(self->timeEvents);
    for (int i = 0; i < count; ++i) {
        const TimeEvent *event = CFArrayGetValueAtIndex(self->timeEvents, i);
        if (tick <= event->__.tick) {
            break;
        }

        int32_t resolution = self->resolution * 4 / denominator;
        int32_t measureLength = resolution * numerator;

        measure += (event->__.tick - offsetTick) / measureLength;
        offsetTick = event->__.tick;

        numerator = event->numerator;
        denominator = event->denominator;
    }

    tick -= offsetTick;

    int32_t resolution = self->resolution * 4 / denominator;
    int32_t measureLength = resolution * numerator;
    
    ret.m = measure + tick / measureLength;
    ret.b = (tick % measureLength) / resolution + 1;
    ret.t = tick % resolution;

    return ret;
}

int32_t TimeTableMicroSec2Tick(TimeTable *self, int64_t usec)
{
    float offsetTick = 0;
    float usecPerTick = 60 * 1000 * 1000 / 120.0f / self->resolution;

    CFIndex count = CFArrayGetCount(self->tempoEvents);
    for (int i = 0; i < count; ++i) {
        const TempoEvent *event = CFArrayGetValueAtIndex(self->tempoEvents, i);
        float tick = offsetTick + usec / usecPerTick;
        if (tick < event->__.tick) {
            break;
        }

        usec -= (event->__.tick - offsetTick) * usecPerTick;
        usecPerTick = 60 * 1000 * 1000 / event->tempo / self->resolution;
        offsetTick = event->__.tick;
    }

    return roundf(offsetTick + usec / usecPerTick);
}

int64_t TimeTableTick2MicroSec(TimeTable *self, int32_t tick)
{
    float offsetTick = 0;
    float usecPerTick = 60 * 1000 * 1000 / 120.0f / self->resolution;
    float usec = 0;

    CFIndex count = CFArrayGetCount(self->tempoEvents);
    for (int i = 0; i < count; ++i) {
        const TempoEvent *event = CFArrayGetValueAtIndex(self->tempoEvents, i);
        if (tick < event->__.tick) {
            break;
        }

        usec += (event->__.tick - offsetTick) * usecPerTick;
        usecPerTick = 60 * 1000 * 1000 / event->tempo / self->resolution;
        offsetTick = event->__.tick;
    }

    return roundf(usec + (tick - offsetTick) * usecPerTick);
}

void TimeTableGetTempoByTick(TimeTable *self, int32_t tick, float *tempo)
{
    const TempoEvent *targetEvent = NULL;
    CFIndex count = CFArrayGetCount(self->tempoEvents);
    for (int i = 0; i < count; ++i) {
        const TempoEvent *event = CFArrayGetValueAtIndex(self->tempoEvents, i);
        if (tick < event->__.tick) {
            break;
        }

        targetEvent = event;
    }

    *tempo = targetEvent ? targetEvent->tempo : 120.0f;
}

void TimeTableGetTimeSignByTick(TimeTable *self, int32_t tick, int16_t *numerator, int16_t *denominator)
{
    const TimeEvent *targetEvent = NULL;
    CFIndex count = CFArrayGetCount(self->timeEvents);
    for (int i = 0; i < count; ++i) {
        const TimeEvent *event = CFArrayGetValueAtIndex(self->timeEvents, i);
        if (tick < event->__.tick) {
            break;
        }

        targetEvent = event;
    }

    *numerator = targetEvent ? targetEvent->numerator : 4;
    *denominator = targetEvent ? targetEvent->denominator : 4;
}

static void *__TimeTableInit(void *_self, ...)
{
    TimeTable *self = _self;
    self->timeEvents = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    self->tempoEvents = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    self->resolution = 480;
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
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<TimeTable: resolution=%d timeEvents=%@ tempoEvents=%@>"), self->resolution, self->timeEvents, self->tempoEvents);
}

static void __TimeTableAccept(void *self, void *visitor)
{
    NAVtbl(visitor, SequenceVisitor)->visitTimeTable(visitor, self);
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
NADeclareVtbl(TimeTable, SequenceElement, __TimeTableAccept);
NADeclareClass(TimeTable, NAType, SequenceElement);


static void *__MidiEventInit(void *_self, ...)
{
    MidiEvent *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->tick = va_arg(ap, int32_t);
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

    MidiEvent *copied = (MidiEvent *)__MidiEventInit(NATypeAlloc(self->__.clazz), self->tick);
    memcpy(&copied->tick, &self->tick, self->__.clazz->size - sizeof(NAType));
    return copied;
}

NADeclareVtbl(MidiEvent, NAType, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
NADeclareClass(MidiEvent, NAType);


static void *__TimeEventDescription(const void *_self)
{
    const TimeEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<TimeEvent: tick=%d numerator=%d denominator=%d>"), self->__.tick, self->numerator, self->denominator);
}

static void __TimeEventAccept(void *self, void *visitor)
{
    NAVtbl(visitor, SequenceVisitor)->visitTimeEvent(visitor, self);
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
NADeclareVtbl(TimeEvent, SequenceElement, __TimeEventAccept);
NADeclareClass(TimeEvent, NAType, SequenceElement);


static void *__TempoEventDescription(const void *_self)
{
    const TempoEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<TempoEvent: tick=%d tempo=%f>"), self->__.tick, self->tempo);
}

static void __TempoEventAccept(void *self, void *visitor)
{
    NAVtbl(visitor, SequenceVisitor)->visitTempoEvent(visitor, self);
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
NADeclareVtbl(TempoEvent, SequenceElement, __TempoEventAccept);
NADeclareClass(TempoEvent, NAType, SequenceElement);


static void __MarkerEventDestroy(void *_self)
{
    MarkerEvent *self = _self;
    if (self->text) {
        CFRelease(self->text);
    }
}

void *__MarkerEventCopy(const void *_self)
{
    const MarkerEvent *self = _self;

    MarkerEvent *copied = (MarkerEvent *)__MidiEventInit(NATypeAlloc(self->__.__.clazz), self->__.tick);
    copied->text = CFStringCreateCopy(NULL, self->text);
    return copied;
}

static void *__MarkerEventDescription(const void *_self)
{
    const MarkerEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<MarkerEvent: tick=%d text=%@>"), self->__.tick, self->text);
}

static void __MarkerEventAccept(void *self, void *visitor)
{
    NAVtbl(visitor, SequenceVisitor)->visitMarkerEvent(visitor, self);
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
NADeclareVtbl(MarkerEvent, SequenceElement, __MarkerEventAccept);
NADeclareClass(MarkerEvent, NAType, SequenceElement);


static void *__SoundSelectEventDescription(const void *_self)
{
    const SoundSelectEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<SoundSelectEvent: tick=%d channel=%d msb=%d lsb=%d programNo=%d>"), self->__.tick, self->channel, self->msb, self->lsb, self->programNo);
}

static void __SoundSelectEventAccept(void *self, void *visitor)
{
    NAVtbl(visitor, SequenceVisitor)->visitSoundSelectEvent(visitor, self);
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
NADeclareVtbl(SoundSelectEvent, SequenceElement, __SoundSelectEventAccept);
NADeclareClass(SoundSelectEvent, NAType, SequenceElement);


static void *__NoteEventDescription(const void *_self)
{
    const NoteEvent *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<NoteEvent: tick=%d channel=%d noteNo=%d velocity=%d gatetime=%d>"), self->__.tick, self->channel, self->noteNo, self->velocity, self->gatetime);
}

static void __NoteEventAccept(void *self, void *visitor)
{
    NAVtbl(visitor, SequenceVisitor)->visitNoteEvent(visitor, self);
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
NADeclareVtbl(NoteEvent, SequenceElement, __NoteEventAccept);
NADeclareClass(NoteEvent, NAType, SequenceElement);

void SequenceElementAccept(void *self, void *visitor)
{
    NAVtbl(self, SequenceElement)->accept(self, visitor);
}
