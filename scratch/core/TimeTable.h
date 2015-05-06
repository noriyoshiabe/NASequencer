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

extern TimeTable *TimeTableCreate();
extern TimeTable *TimeTableCreateFromTimeTable(TimeTable *from, int32_t tick);
extern TimeTable *TimeTableRetain(TimeTable *self);
extern void TimeTableRelease(TimeTable *self);
extern void TimeTableDump(TimeTable *self);
extern void TimeTableDumpToBuffer(TimeTable *self, char *buffer, size_t size);

extern void TimeTableAddTimeSign(TimeTable *self, int32_t tick, TimeSign timeSign);
extern void TimeTableAddTempo(TimeTable *self, int32_t tick, float tempo);

extern int32_t TimeTableResolution(TimeTable *self);
extern int32_t TimeTableLength(TimeTable *self);
extern int32_t TimeTableTickByMeasure(TimeTable *self, int32_t measure);
extern TimeSign TimeTableTimeSignOnTick(TimeTable *self, int32_t tick);
extern float TimeTableTempoOnTick(TimeTable *self, int32_t tick);
extern Location TimeTableTick2Location(TimeTable *self, int32_t tick);
extern int64_t TimeTableTick2MicroSec(TimeTable *self, int32_t tick);
extern int32_t TimeTableMicroSec2Tick(TimeTable *self, int64_t usec);

extern const Location LocationZero;
