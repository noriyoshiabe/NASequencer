#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct _TimeSign {
    int16_t numerator;
    int16_t denominator;
} TimeSign;

typedef struct _TimeEvent {
    int32_t tick;
    TimeSign timeSign;
} TimeEvent;

typedef struct _TempoEvent {
    int32_t tick;
    float tempo;
} TempoEvent;


typedef struct _TimeTable TimeTable;

TimeTable *TimeTableCreate();
TimeTable *TimeTableCreateFromTimeTable(TimeTable *from, int32_t tick);
void TimeTableDestroy(TimeTable *self);
void TimeTableDump(TimeTable *self);

void TimeTableAddTimeSign(TimeTable *self, int32_t tick, TimeSign timeSign);
void TimeTableAddTempo(TimeTable *self, int32_t tick, float tempo);

int32_t TimeTableResolution(TimeTable *self);
int32_t TimeTableTickByMeasure(TimeTable *self, int32_t measure);
TimeSign TimeTableTimeSignOnTick(TimeTable *self, int32_t tick);
float TimeTableTempoOnTick(TimeTable *self, int32_t tick);

bool TimeTableSetResolution(TimeTable *self, int32_t resolution);

size_t TimeTableGetTimeSignCount(TimeTable *self);
size_t TimeTableGetTempoCount(TimeTable *self);

void TimeTableGetTimeSignValues(TimeTable *self, TimeEvent **values);
void TimeTableGetTempoValues(TimeTable *self, TempoEvent **values);
