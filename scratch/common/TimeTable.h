#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct _TimeSign {
    int16_t numerator;
    int16_t denominator;
} TimeSign;

typedef struct _TimeTable TimeTable;

TimeTable *TimeTableCreate();
TimeTable *TimeTableCreateFromTimeTable(TimeTable *from, int32_t tick);
void TimeTableDestroy(TimeTable *self);
void TimeTableDump(TimeTable *self);
void TimeTableAddTimeSign(TimeTable *self, int32_t tick, TimeSign timeSign);

int32_t TimeTableResolution(TimeTable *self);
int32_t TimeTableTickByMeasure(TimeTable *self, int32_t measure);
TimeSign TimeTableTimeSignOnTick(TimeTable *self, int32_t tick);

bool TimeTableHasResolution(TimeTable *self);
bool TimeTableHasTimeSign(TimeTable *self);

void TimeTableSetResolution(TimeTable *self, int32_t resolution);
