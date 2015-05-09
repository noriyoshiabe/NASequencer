#include "TimeTable.h"
#include "TimeTable_internal.h"

#include <stdlib.h>
#include <string.h>
#include <CoreFoundation/CoreFoundation.h>

struct _TimeTable {
    int refCount;
    uint16_t resolution;
    bool resolutionChanged;
    CFMutableArrayRef timeList;
    CFMutableArrayRef tempoList;
    int32_t length;
};

const Location LocationZero = {1, 1, 0};


static TimeEvent *TimeEventCreate(int32_t tick, TimeSign timeSign)
{
    TimeEvent *ret = calloc(1, sizeof(TimeEvent));
    ret->tick = tick;
    ret->timeSign = timeSign;
    return ret;
}

static TempoEvent *TempoEventCreate(int32_t tick, float tempo)
{
    TempoEvent *ret = calloc(1, sizeof(TempoEvent));
    ret->tick = tick;
    ret->tempo = tempo;
    return ret;
}


static void TimeTableEventReleaseCallbak(CFAllocatorRef allocator, const void *value)
{
    free((void *)value);
}

static const CFArrayCallBacks TimeTableEventListCallbacks = {0, NULL, TimeTableEventReleaseCallbak, NULL, NULL};

static CFComparisonResult TimeTableEventComparator(const void *val1, const void *val2, void *context)
{
    return *((int32_t *)val1) - *((int32_t *)val2);
}

TimeTable *TimeTableCreate()
{
    TimeTable *ret = calloc(1, sizeof(TimeTable));
    ret->refCount = 1;
    ret->resolution = 480;
    ret->timeList = CFArrayCreateMutable(NULL, 0, &TimeTableEventListCallbacks);
    ret->tempoList = CFArrayCreateMutable(NULL, 0, &TimeTableEventListCallbacks);

    TimeSign timeSign = {4, 4};
    TimeTableAddTimeSign(ret, 0, timeSign);
    TimeTableAddTempo(ret, 0, 120.0);
    return ret;
}

TimeTable *TimeTableCreateFromTimeTable(TimeTable *from, int32_t tick)
{
    TimeTable *ret = TimeTableCreate();
    ret->resolution = from->resolution;

    TimeSign timeSign = TimeTableTimeSignOnTick(from, tick);
    TimeTableAddTimeSign(ret, 0, timeSign);
    float tempo = TimeTableTempoOnTick(from, tick);
    TimeTableAddTempo(ret, 0, tempo);
    return ret;
}

TimeTable *TimeTableRetain(TimeTable *self)
{
    ++self->refCount;
    return self;
}

void TimeTableRelease(TimeTable *self)
{
    if (0 == --self->refCount) {
        CFRelease(self->timeList);
        CFRelease(self->tempoList);
        free(self);
    }
}

void TimeTableDump(TimeTable *self)
{
    CFIndex count;

    printf("\n----- time table -----\n");
    printf("resolution=%d length=%d\n", self->resolution, self->length);

    count = CFArrayGetCount(self->timeList);
    for (CFIndex i = 0; i < count; ++i) {
        const TimeEvent *timeEvent = CFArrayGetValueAtIndex(self->timeList, i);
        printf("[TimeSign] tick=%d numerator=%d denominator=%d\n", timeEvent->tick, timeEvent->timeSign.numerator, timeEvent->timeSign.denominator);
    }

    count = CFArrayGetCount(self->tempoList);
    for (CFIndex i = 0; i < count; ++i) {
        const TempoEvent *tempoEvent = CFArrayGetValueAtIndex(self->tempoList, i);
        printf("[Tempo] tick=%d tempo=%.2f\n", tempoEvent->tick, tempoEvent->tempo);
    }

    printf("----- time table -----\n");
}

void TimeTableDumpToBuffer(TimeTable *self, char *buffer, size_t size)
{
    CFMutableStringRef cfString = CFStringCreateMutable(NULL, 0);

    CFIndex count;

    CFStringAppendFormat(cfString, NULL, CFSTR("\n----- time table -----\n"));
    CFStringAppendFormat(cfString, NULL, CFSTR("resolution=%d length=%d\n"), self->resolution, self->length);

    count = CFArrayGetCount(self->timeList);
    for (CFIndex i = 0; i < count; ++i) {
        const TimeEvent *timeEvent = CFArrayGetValueAtIndex(self->timeList, i);
        CFStringAppendFormat(cfString, NULL, CFSTR("[TimeSign] tick=%d numerator=%d denominator=%d\n"), timeEvent->tick, timeEvent->timeSign.numerator, timeEvent->timeSign.denominator);
    }

    count = CFArrayGetCount(self->tempoList);
    for (CFIndex i = 0; i < count; ++i) {
        const TempoEvent *tempoEvent = CFArrayGetValueAtIndex(self->tempoList, i);
        CFStringAppendFormat(cfString, NULL, CFSTR("[Tempo] tick=%d tempo=%.2f\n"), tempoEvent->tick, tempoEvent->tempo);
    }

    CFStringAppendFormat(cfString, NULL, CFSTR("----- time table -----\n"));
    CFStringGetCString(cfString, buffer, size, kCFStringEncodingUTF8);
    CFRelease(cfString);
}

void TimeTableAddTimeSign(TimeTable *self, int32_t tick, TimeSign timeSign)
{
    TimeEvent *event = TimeEventCreate(tick, timeSign);

    CFIndex count = CFArrayGetCount(self->timeList);
    CFIndex index = CFArrayBSearchValues(self->timeList, CFRangeMake(0, count), event, TimeTableEventComparator, NULL);
    if (index < count) {
        CFArrayRemoveValueAtIndex(self->timeList, index);
    }

    CFArrayAppendValue(self->timeList, event);
    CFArraySortValues(self->timeList, CFRangeMake(0, CFArrayGetCount(self->timeList)), TimeTableEventComparator, NULL);
}

void TimeTableAddTempo(TimeTable *self, int32_t tick, float tempo)
{
    TempoEvent *event = TempoEventCreate(tick, tempo);

    CFIndex count = CFArrayGetCount(self->tempoList);
    CFIndex index = CFArrayBSearchValues(self->tempoList, CFRangeMake(0, count), event, TimeTableEventComparator, NULL);
    if (index < count) {
        CFArrayRemoveValueAtIndex(self->tempoList, index);
    }

    CFArrayAppendValue(self->tempoList, event);
    CFArraySortValues(self->tempoList, CFRangeMake(0, CFArrayGetCount(self->tempoList)), TimeTableEventComparator, NULL);
}

int32_t TimeTableResolution(TimeTable *self)
{
    return self->resolution;
}

int32_t TimeTableLength(TimeTable *self)
{
    return self->length;
}

int32_t TimeTableTickByMeasure(TimeTable *self, int32_t measure)
{
    int32_t offsetTick = 0;
    int32_t tickPerMeasure = self->resolution * 4;

    measure -= 1;

    CFIndex count = CFArrayGetCount(self->timeList);
    for (int i = 0; i < count; ++i) {
        const TimeEvent *timeEvent = CFArrayGetValueAtIndex(self->timeList, i);

        int32_t tick = tickPerMeasure * measure + offsetTick;
        if (tick < timeEvent->tick) {
            break;
        }

        measure -= timeEvent->tick / tickPerMeasure;
        tickPerMeasure = self->resolution * 4 / timeEvent->timeSign.denominator * timeEvent->timeSign.numerator;
        offsetTick = timeEvent->tick;
    }

    return tickPerMeasure * measure + offsetTick;
}

TimeSign TimeTableTimeSignOnTick(TimeTable *self, int32_t tick)
{
    TimeSign ret = {4, 4};

    CFIndex count = CFArrayGetCount(self->timeList);
    for (CFIndex i = 0; i < count; ++i) {
        TimeEvent *test = (TimeEvent *)CFArrayGetValueAtIndex(self->timeList, i);
        if (tick < test->tick) {
            break;
        }

        ret = test->timeSign;
    }

    return ret;
}

float TimeTableTempoOnTick(TimeTable *self, int32_t tick)
{
    float ret = 120.0;

    CFIndex count = CFArrayGetCount(self->tempoList);
    for (CFIndex i = 0; i < count; ++i) {
        TempoEvent *test = (TempoEvent *)CFArrayGetValueAtIndex(self->tempoList, i);
        if (tick < test->tick) {
            break;
        }

        ret = test->tempo;
    }

    return ret;
}

Location TimeTableTick2Location(TimeTable *self, int32_t tick)
{
    Location ret = {0};
    TimeSign timeSign = {4, 4};

    int32_t offsetTick = 0;
    int32_t measure = 1;

    CFIndex count = CFArrayGetCount(self->timeList);
    for (int i = 0; i < count; ++i) {
        const TimeEvent *event = CFArrayGetValueAtIndex(self->timeList, i);
        if (tick <= event->tick) {
            break;
        }

        int32_t resolution = self->resolution * 4 / timeSign.denominator;
        int32_t measureLength = resolution * timeSign.numerator;

        measure += (event->tick - offsetTick) / measureLength;
        offsetTick = event->tick;

        timeSign = event->timeSign;
    }

    tick -= offsetTick;

    int32_t resolution = self->resolution * 4 / timeSign.denominator;
    int32_t measureLength = resolution * timeSign.numerator;
    
    ret.m = measure + tick / measureLength;
    ret.b = (tick % measureLength) / resolution + 1;
    ret.t = tick % resolution;

    return ret;
}

int64_t TimeTableTick2MicroSec(TimeTable *self, int32_t tick)
{
    float offsetTick = 0;
    float usecPerTick = 60 * 1000 * 1000 / 120.0f / self->resolution;
    float usec = 0;

    CFIndex count = CFArrayGetCount(self->tempoList);
    for (int i = 0; i < count; ++i) {
        const TempoEvent *event = CFArrayGetValueAtIndex(self->tempoList, i);
        if (tick < event->tick) {
            break;
        }

        usec += (event->tick - offsetTick) * usecPerTick;
        usecPerTick = 60 * 1000 * 1000 / event->tempo / self->resolution;
        offsetTick = event->tick;
    }

    return roundf(usec + (tick - offsetTick) * usecPerTick);
}

int32_t TimeTableMicroSec2Tick(TimeTable *self, int64_t usec)
{
    float offsetTick = 0;
    float usecPerTick = 60 * 1000 * 1000 / 120.0f / self->resolution;

    CFIndex count = CFArrayGetCount(self->tempoList);
    for (int i = 0; i < count; ++i) {
        const TempoEvent *event = CFArrayGetValueAtIndex(self->tempoList, i);
        float tick = offsetTick + usec / usecPerTick;
        if (tick < event->tick) {
            break;
        }

        usec -= (event->tick - offsetTick) * usecPerTick;
        usecPerTick = 60 * 1000 * 1000 / event->tempo / self->resolution;
        offsetTick = event->tick;
    }

    return roundf(offsetTick + usec / usecPerTick);
}

bool TimeTableSetResolution(TimeTable *self, int32_t resolution)
{
    if (self->resolutionChanged) {
        return false;
    }

    if (resolution < 1 || 9600 < resolution) {
        return false;
    }

    self->resolution = resolution;
    self->resolutionChanged = true;
    return true;
}

void TimeTableSetLength(TimeTable *self, int32_t length)
{
    self->length = length;
}

size_t TimeTableGetTimeSignCount(TimeTable *self)
{
    return CFArrayGetCount(self->timeList);
}

size_t TimeTableGetTempoCount(TimeTable *self)
{
    return CFArrayGetCount(self->tempoList);
}

void TimeTableGetTimeSignValues(TimeTable *self, TimeEvent **values)
{
    CFArrayGetValues(self->timeList, CFRangeMake(0, CFArrayGetCount(self->timeList)), (const void **)values);
}

void TimeTableGetTempoValues(TimeTable *self, TempoEvent **values)
{
    CFArrayGetValues(self->tempoList, CFRangeMake(0, CFArrayGetCount(self->tempoList)), (const void **)values);
}
