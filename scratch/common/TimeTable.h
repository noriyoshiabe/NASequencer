#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct _TimeSign {
    int32_t tick;
    int16_t numerator;
    int16_t denominator;
} TimeSign;

TimeSign *TimeSignCreate(int32_t tick, int16_t numerator, int16_t denominator);
TimeSign *TimeSignCreateWithTimeSign(int32_t tick, const TimeSign *from);
void TimeSignDestroy(TimeSign *self);

typedef struct _TimeTable TimeTable;

TimeTable *TimeTableCreate();
void TimeTableDestroy(TimeTable *self);
void TimeTableAddTimeSign(TimeTable *self, const TimeSign *timeSign);

int32_t TimeTableResolution(TimeTable *self);
int32_t TimeTableTickByMeasure(TimeTable *self, int32_t measure);
TimeSign *TimeTableTimeSignOnTick(TimeTable *self, int32_t tick);

bool TimeTableHasResolution(TimeTable *self);
bool TimeTableHasTimeSign(TimeTable *self);

void TimeTableSetResolution(TimeTable *self, int32_t resolution);
