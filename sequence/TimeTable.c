#include "TimeTable.h"
#include "NAArray.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct _TimeSignRecord {
    int32_t tickStart;
    int32_t tickEnd;
    int16_t numerator;
    int16_t denominator;
    int32_t measureStart;
    int32_t measureEnd;
    int32_t measureLength;
} TimeSignRecord;

typedef struct _TempoRecord {
    int32_t tickStart;
    int32_t tickEnd;
    float tempo;
    int64_t usecStart;
    int64_t usecEnd;
} TempoRecord;

struct _TimeTable {
    int32_t resolution;
    int32_t length;
    NAArray *timeSignRecords;
    NAArray *tempoRecords;
};

static int TimeSignRecordFindByTickComparator(const void *_tick, const void *_record);
static int TimeSignRecordFindByMeasureComparator(const void *_measure, const void *_record);
static int TempoRecordFindByTickComparator(const void *_tick, const void *_record);
static int TempoRecordFindByUsecComparator(const void *_usec, const void *_record);

static void TimeTableRefreshMeasureFrom(TimeTable *self, int index);
static void TimeTableRefreshUsecFrom(TimeTable *self, int index);

const Location LocationZero = {1, 1, 0};

TimeTable *TimeTableCreate()
{
    TimeTable *self = calloc(1, sizeof(TimeTable));
    self->resolution = 480;
    self->timeSignRecords = NAArrayCreate(4, sizeof(TimeSignRecord));
    self->tempoRecords = NAArrayCreate(4, sizeof(TempoRecord));
    TimeSignRecord timeSign = {0, INT32_MAX, 4, 4, 1, INT32_MAX};
    NAArrayAppend(self->timeSignRecords, &timeSign);
    TempoRecord tempo = {0, INT32_MAX, 120.0, 1, INT64_MAX};
    NAArrayAppend(self->tempoRecords, &tempo);
    return self;
}

void TimeTableDestroy(TimeTable *self)
{
    NAArrayDestroy(self->timeSignRecords);
    NAArrayDestroy(self->tempoRecords);
    free(self);
}

bool TimeTableAddTimeSign(TimeTable *self, int32_t tick, TimeSign timeSign)
{
    Location l = TimeTableTick2Location(self, tick);
    if (1 != l.b || 0 != l.t) {
        return false;
    }

    int count = NAArrayCount(self->timeSignRecords);
    TimeSignRecord *records = NAArrayGetValues(self->timeSignRecords);

    int i = NAArrayFindIndex(self->timeSignRecords, &tick, TimeSignRecordFindByTickComparator);
    if (0 == memcmp(&records[i].timeSign, &timeSign, sizeof(TimeSign))) {
        return true;
    }

    if (records[i].tickStart == tick) {
        records[i].timeSign = timeSign;
        return true;
    }

    if (i + 1 < count) {
        if (0 == memcmp(&records[i + 1].timeSign, &timeSign, sizeof(TimeSign))) {
            records[i + 1].tickStart = tick;
            return true;
        }
    }

    TimeTableRecord insert;
    memcpy(&insert, &records[i], sizeof(TimeSignRecord));
    records[i].tickEnd = tick;
    insert.tickStart = tick;
    insert.timeSign = timeSign;

    if (i + 1 < count) {
        NAArrayInsertAt(self->timeSignRecords, i + 1, &insert);
    }
    else {
        NAArrayAppend(self->timeSignRecords, &insert);
    }

    TimeTableRefreshMeasureFrom(self, i);
    return true;
}

bool TimeTableAddTempo(TimeTable *self, int32_t tick, float tempo)
{
    int count = NAArrayCount(self->tempoRecords);
    TempoRecord *records = NAArrayGetValues(self->tempoRecords);

    int i = NAArrayFindIndex(self->tempoRecords, &tick, TempoRecordFindByTickComparator);
    if (records[i].tempo == tempo) {
        return true;
    }

    if (records[i].tickStart == tick) {
        records[i].tempo = tempo;
        return true;
    }

    if (i + 1 < count) {
        if (records[i + 1].tempo == tempo) {
            records[i + 1].tickStart = tick;
            return true;
        }
    }

    TempoRecord insert;
    memcpy(&insert, &records[i], sizeof(TempoRecord));
    records[i].tickEnd = tick;
    insert.tickStart = tick;
    insert.tempo = tempo;

    if (i + 1 < count) {
        NAArrayInsertAt(self->tempoRecords, i + 1, &insert);
    }
    else {
        NAArrayAppend(self->tempoRecords, &insert);
    }

    TimeTableRefreshUsecFrom(self, i);
    return true;
}

void TimeTableSetLength(TimeTable *self, int32_t length)
{
    self->length = length;
}


int32_t TimeTableResolution(TimeTable *self)
{
    return self->resolution;
}

int32_t TimeTableLength(TimeTable *self)
{
    return self->length;
}

int32_t TimeTableTickByMeasure(TimeTable *self, int32_t measure)
{
    int count = NAArrayCount(self->timeSignRecords);
    TimeSignRecord *records = NAArrayGetValues(self->timeSignRecords);
    int i = NAArrayFindIndex(self->timeSignRecords, &measure, TimeSignRecordFindByMeasureComparator);

    return records[i].tickStart + records[i].measureLength * (measure - records[i].measureStart);
}

int32_t TimeTableTickByLocation(TimeTable *self, Location location)
{
    int count = NAArrayCount(self->timeSignRecords);
    TimeSignRecord *records = NAArrayGetValues(self->timeSignRecords);
    int i = NAArrayFindIndex(self->timeSignRecords, &location.m, TimeSignRecordFindByMeasureComparator);

    int32_t ret = records[i].tickStart + records[i].measureLength * (location.m - records[i].measureStart);
    ret += (location.b - 1) * self->resolution * 4 / records[i].denominator * records[i].numerator;
    ret += location.t;
    return ret;
}

TimeSign TimeTableTimeSignOnTick(TimeTable *self, int32_t tick)
{
    int count = NAArrayCount(self->timeSignRecords);
    TimeSignRecord *records = NAArrayGetValues(self->timeSignRecords);
    int i = NAArrayFindIndex(self->timeSignRecords, &tick, TimeSignRecordFindByTickComparator);
    return records[i].timeSign;
}

float TimeTableTempoOnTick(TimeTable *self, int32_t tick)
{
    int count = NAArrayCount(self->tempoRecords);
    TempoRecord *records = NAArrayGetValues(self->tempoRecords);
    int i = NAArrayFindIndex(self->tempoRecords, &tick, TempoRecordFindByTickComparator);
    return records[i].tempo;
}

Location TimeTableTick2Location(TimeTable *self, int32_t tick)
{
    int count = NAArrayCount(self->timeSignRecords);
    TimeSignRecord *records = NAArrayGetValues(self->timeSignRecords);
    int i = NAArrayFindIndex(self->timeSignRecords, &tick, TimeSignRecordFindByTickComparator);

    int32_t tickFromPreviousTimeSign = (tick - records[i].tickStart);
    int32_t tickPerBeat = self->resolution * 4 / records[i].timeSign.denominator;

    Location ret;
    ret.m = records[i].measureStart + tickFromPreviousTimeSign / records[i].measureLength;
    ret.b = (tickFromPreviousTimeSign % records[i].measureLength) / tickPerBeat + 1;
    ret.t = tick % tickPerBeat;

    return ret;
}

int64_t TimeTableTick2MicroSec(TimeTable *self, int32_t tick)
{
    int count = NAArrayCount(self->tempoRecords);
    TempoRecord *records = NAArrayGetValues(self->tempoRecords);
    int i = NAArrayFindIndex(self->tempoRecords, &tick, TempoRecordFindByTickComparator);

    double usecPerTick = 60 * 1000 * 1000 / records[i].tempo / self->resolution;
    return round(records[i].usecStart + (tick - records[i].tickStart) * usecPerTick);
}

int32_t TimeTableMicroSec2Tick(TimeTable *self, int64_t usec)
{
    int count = NAArrayCount(self->tempoRecords);
    TempoRecord *records = NAArrayGetValues(self->tempoRecords);
    int i = NAArrayFindIndex(self->tempoRecords, &usec, TempoRecordFindByUsecComparator);

    double usecPerTick = 60 * 1000 * 1000 / records[i].tempo / self->resolution;
    return round(records[i].tickStart + usec / usecPerTick);
}

static void TimeTableRefreshMeasureFrom(TimeTable *self, int index)
{
    int count = NAArrayCount(self->timeSignRecords);
    TimeSignRecord *records = NAArrayGetValues(self->timeSignRecords);

    TimeSignRecord *previous = 0 < i ? &records[i - 1] : &((TimeSignRecord){0});
    for (int i = index ; i < count; ++i) {
        records[i].measureStart = previous->measureEnd;
        records[i].measureLength = self->resolution * 4 / records[i].denominator * records[i].numerator;
        records[i].measureEnd = records[i].measureStart + (records[i].tickEnd - records[i].tickStart) / records[i].measureLength;
        previous = &records[i];
    }
}

static void TimeTableRefreshUsecFrom(TimeTable *self, int index)
{
    int count = NAArrayCount(self->tempoRecords);
    TempoRecord *records = NAArrayGetValues(self->tempoRecords);

    TempoRecord *previous = 0 < i ? &records[i - 1] : &((TempoRecord){0});
    for (int i = index; i < count; ++i) {
        double usecPerTick = 60 * 1000 * 1000 / records[i].tempo / self->resolution;
        records[i].usecStart = previous->usecEnd;
        records[i].usecEnd = records[i].usecStart + round((records[i].tickEnd - records[i].tickStart) / usecPerTick);
        previous = &records[i];
    }
}

static int TimeTableRecordFindByTickComparator(const void *_tick, const void *_record)
{
    const int32_t *tick = _tick;
    const TimeSignRecord *record = _record;

    if (record->tickStart <= *tick && *tick < record->tickEnd) {
        return 0;
    }
    else {
        return *tick - record->tickStart;
    }
}

static int TimeSignRecordFindByMeasureComparator(const void *_measure, const void *_record)
{
    const int32_t *measure = _measure;
    const TimeTableRecord *record = _record;

    if (record->measureStart <= *measure && *measure < record->measureEnd) {
        return 0;
    }
    else {
        return *measure - record->measureStart;
    }
}

static int TempoRecordFindByTickComparator(const void *_tick, const void *_record)
{
    const int32_t *tick = _tick;
    const TempoRecord *record = _record;

    if (record->tickStart <= *tick && *tick < record->tickEnd) {
        return 0;
    }
    else {
        return *tick - record->tickStart;
    }
}

static int TempoRecordFindByUsecComparator(const void *_usec, const void *_record)
{
    const int64_t *tick = _tick;
    const TempoRecord *record = _record;

    if (record->usecStart <= *tick && *tick < record->usecEnd) {
        return 0;
    }
    else {
        return *usec - record->usecStart;
    }
}
