#include "TimeTable.h"

#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct _TimeEvent {
    int32_t tick;
    TimeSign timeSign;
} TimeEvent;

static TimeEvent *TimeEventCreate(int32_t tick, TimeSign timeSign)
{
    TimeEvent *ret = calloc(1, sizeof(TimeEvent));
    ret->tick = tick;
    ret->timeSign = timeSign;
    return ret;
}

static void TimeEventDestroy(TimeEvent *self)
{
    free(self);
}


struct _TimeTable {
    int32_t resolution;
    CFMutableArrayRef timeList;
};

static void TimeEventReleaseCallbak(CFAllocatorRef allocator, const void *value)
{
    TimeEventDestroy((TimeEvent *)value);
}
static const CFArrayCallBacks TimeTableTimeListCallbacks = {0, NULL, TimeEventReleaseCallbak, NULL, NULL};

static CFComparisonResult TimeTableTimeEventComparator(const void *val1, const void *val2, void *context)
{
    return ((TimeEvent *)val1)->tick - ((TimeEvent *)val2)->tick;
}

TimeTable *TimeTableCreate()
{
    TimeTable *ret = calloc(1, sizeof(TimeTable));
    ret->timeList = CFArrayCreateMutable(NULL, 0, &TimeTableTimeListCallbacks);
    return ret;
}

TimeTable *TimeTableCreateFromTimeTable(TimeTable *from, int32_t tick)
{
    TimeTable *ret = TimeTableCreate();
    ret->resolution = from->resolution;

    TimeSign timeSign = TimeTableTimeSignOnTick(from, tick);
    TimeTableAddTimeSign(ret, 0, timeSign);
    return ret;
}

void TimeTableDestroy(TimeTable *self)
{
    CFRelease(self->timeList);
    free(self);
}

void TimeTableAddTimeSign(TimeTable *self, int32_t tick, TimeSign timeSign)
{
    CFArrayAppendValue(self->timeList, TimeEventCreate(tick, timeSign));
    CFArraySortValues(self->timeList, CFRangeMake(0, CFArrayGetCount(self->timeList)), TimeTableTimeEventComparator, NULL);
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

bool TimeTableHasResolution(TimeTable *self)
{
    return 0 != self->resolution;
}

bool TimeTableHasTimeSign(TimeTable *self)
{
    return 0 < CFArrayGetCount(self->timeList);
}

void TimeTableSetResolution(TimeTable *self, int32_t resolution)
{
    self->resolution = resolution;
}
