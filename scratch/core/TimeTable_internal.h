#pragma once

#include <stdint.h>

typedef struct _TimeEvent {
    int32_t tick;
    TimeSign timeSign;
} TimeEvent;

typedef struct _TempoEvent {
    int32_t tick;
    float tempo;
} TempoEvent;

size_t TimeTableGetTimeSignCount(TimeTable *self);
size_t TimeTableGetTempoCount(TimeTable *self);

void TimeTableGetTimeSignValues(TimeTable *self, TimeEvent **values);
void TimeTableGetTempoValues(TimeTable *self, TempoEvent **values);

bool TimeTableSetResolution(TimeTable *self, int32_t resolution);
void TimeTableSetLength(TimeTable *self, int32_t length);
