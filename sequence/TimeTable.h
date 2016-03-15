#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "NAArray.h"

typedef struct _Location {
    int32_t m;
    int16_t b;
    int16_t t;
} Location;

typedef struct _TimeSign {
    int16_t numerator;
    int16_t denominator;
} TimeSign;

typedef struct _RepeatSection {
    int32_t tickStart;
    int32_t tickEnd;
} RepeatSection;

typedef struct _TimeTable TimeTable;

extern int32_t TimeTableResolution(TimeTable *self);
extern int32_t TimeTableLength(TimeTable *self);
extern int32_t TimeTableTickByMeasure(TimeTable *self, int32_t measure);
extern int32_t TimeTableTickByLocation(TimeTable *self, Location location);
extern TimeSign TimeTableTimeSignOnTick(TimeTable *self, int32_t tick);
extern float TimeTableTempoOnTick(TimeTable *self, int32_t tick);
extern Location TimeTableTick2Location(TimeTable *self, int32_t tick);
extern int64_t TimeTableTick2MicroSec(TimeTable *self, int32_t tick);
extern int32_t TimeTableMicroSec2Tick(TimeTable *self, int64_t usec);
extern RepeatSection TimeTableRepeatSectionOnTick(TimeTable *self, int32_t tick);
extern NAArray *TimeTableGetRepeatSections(TimeTable *self);

extern const Location LocationZero;

extern TimeTable *TimeTableCreate();
extern void TimeTableDestroy(TimeTable *self);
extern void TimeTableSetResolution(TimeTable *self, int32_t resolution);
extern void TimeTableSetLength(TimeTable *self, int32_t length);
extern bool TimeTableAddTimeSign(TimeTable *self, int32_t tick, TimeSign timeSign);
extern bool TimeTableAddTempo(TimeTable *self, int32_t tick, float tempo);
extern bool TimeTableAddRepeatPoint(TimeTable *self, int32_t tick);

extern void TimeTableDump(TimeTable *self, int indent);
