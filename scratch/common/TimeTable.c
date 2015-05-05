#include "TimeTable.h"

#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>

struct _TimeTable {
    int32_t resolution;
    CFMutableArrayRef timeSignList;
};

TimeSign *TimeSignCreate(int32_t tick, int16_t numerator, int16_t denominator)
{
    TimeSign *ret = calloc(1, sizeof(TimeSign));
    ret->tick = tick;
    ret->numerator = numerator;
    ret->denominator = denominator;
    return ret;
}

TimeSign *TimeSignCreateWithTimeSign(int32_t tick, const TimeSign *from)
{
    TimeSign *ret = calloc(1, sizeof(TimeSign));
    ret->tick = tick;
    ret->numerator = from->numerator;
    ret->denominator = from->denominator;
    return ret;
}

void TimeSignDestroy(TimeSign *self)
{
    free(self);
}


static void TimeSignReleaseCallbak(CFAllocatorRef allocator, const void *value)
{
    TimeSignDestroy((TimeSign *)value);
}
static const CFArrayCallBacks TimeTableTimeSignListCallbacks = {0, NULL, TimeSignReleaseCallbak, NULL, NULL};

static CFComparisonResult TimeTableTimeSignComparator(const void *val1, const void *val2, void *context)
{
    return ((TimeSign *)val1)->tick - ((TimeSign *)val2)->tick;
}

TimeTable *TimeTableCreate()
{
    TimeTable *ret = calloc(1, sizeof(TimeTable));
    ret->timeSignList = CFArrayCreateMutable(NULL, 0, &TimeTableTimeSignListCallbacks);
    return ret;
}

void TimeTableDestroy(TimeTable *self)
{
    CFRelease(self->timeSignList);
    free(self);
}

void TimeTableAddTimeSign(TimeTable *self, const TimeSign *timeSign)
{
    CFArrayAppendValue(self->timeSignList, timeSign);
    CFArraySortValues(self->timeSignList, CFRangeMake(0, CFArrayGetCount(self->timeSignList)), TimeTableTimeSignComparator, NULL);
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

    CFIndex count = CFArrayGetCount(self->timeSignList);
    for (int i = 0; i < count; ++i) {
        const TimeSign *timeSign = CFArrayGetValueAtIndex(self->timeSignList, i);

        int32_t tick = tickPerMeasure * measure + offsetTick;
        if (tick < timeSign->tick) {
            break;
        }

        measure -= timeSign->tick / tickPerMeasure;
        tickPerMeasure = self->resolution * 4 / timeSign->denominator * timeSign->numerator;
        offsetTick = timeSign->tick;
    }

    return tickPerMeasure * measure + offsetTick;
}

TimeSign *TimeTableTimeSignOnTick(TimeTable *self, int32_t tick)
{
    TimeSign *target = NULL;

    CFIndex count = CFArrayGetCount(self->timeSignList);
    for (CFIndex i = 0; i < count; ++i) {
        TimeSign *test = (TimeSign *)CFArrayGetValueAtIndex(self->timeSignList, i);
        if (tick < test->tick) {
            break;
        }

        target = test;
    }

    return target;
}

bool TimeTableHasResolution(TimeTable *self)
{
    return 0 != self->resolution;
}

bool TimeTableHasTimeSign(TimeTable *self)
{
    return 0 < CFArrayGetCount(self->timeSignList);
}

void TimeTableSetResolution(TimeTable *self, int32_t resolution)
{
    self->resolution = resolution;
}
