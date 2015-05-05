#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct _Location {
    int32_t m;
    int16_t b;
    int16_t t;
} Location;

typedef struct _TimeSign {
    int16_t numerator;
    int16_t denominator;
} TimeSign;

typedef struct _TimeTable TimeTable;

TimeTable *TimeTableCreate();
TimeTable *TimeTableCreateFromTimeTable(TimeTable *from, int32_t tick);
TimeTable *TimeTableRetain(TimeTable *self);
void TimeTableRelease(TimeTable *self);
void TimeTableDump(TimeTable *self);
void TimeTableDumpToBuffer(TimeTable *self, char *buffer, size_t size);

void TimeTableAddTimeSign(TimeTable *self, int32_t tick, TimeSign timeSign);
void TimeTableAddTempo(TimeTable *self, int32_t tick, float tempo);

int32_t TimeTableResolution(TimeTable *self);
int32_t TimeTableLength(TimeTable *self);
int32_t TimeTableTickByMeasure(TimeTable *self, int32_t measure);
TimeSign TimeTableTimeSignOnTick(TimeTable *self, int32_t tick);
float TimeTableTempoOnTick(TimeTable *self, int32_t tick);
