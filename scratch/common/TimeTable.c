#include "TimeTable.h"

#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>

struct _TimeTable {
    int32_t resolution;
    bool resolutionChanged;
    CFMutableArrayRef timeList;
    CFMutableArrayRef tempoList;
};


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

void TimeTableDestroy(TimeTable *self)
{
    CFRelease(self->timeList);
    CFRelease(self->tempoList);
    free(self);
}

void TimeTableDump(TimeTable *self)
{
    CFIndex count;

    printf("----- time table -----\n");

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
