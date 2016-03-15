#include "TimeTable.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

typedef struct _TimeSignRecord {
    int32_t tickStart;
    int32_t tickEnd;
    TimeSign timeSign;
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
    NAArray *repeatSections;
};

static int TimeSignRecordFindByTickComparator(const void *_tick, const void *_record);
static int TimeSignRecordFindByMeasureComparator(const void *_measure, const void *_record);
static int TempoRecordFindByTickComparator(const void *_tick, const void *_record);
static int TempoRecordFindByUsecComparator(const void *_usec, const void *_record);
static int RepeatSectionFindByTickComparator(const void *_tick, const void *_section);

static void TimeTableRefreshMeasureFrom(TimeTable *self, int index);
static void TimeTableRefreshUsecFrom(TimeTable *self, int index);

const Location LocationZero = {1, 1, 0};

TimeTable *TimeTableCreate()
{
    TimeTable *self = calloc(1, sizeof(TimeTable));
    self->resolution = 480;
    self->timeSignRecords = NAArrayCreate(4, NULL);
    self->tempoRecords = NAArrayCreate(4, NULL);
    self->repeatSections = NAArrayCreate(4, NULL);

    TimeSignRecord *timeSign = malloc(sizeof(TimeSignRecord));
    *timeSign = (TimeSignRecord){0, INT32_MAX, {4, 4}, 1, INT32_MAX, 480 * 4};
    NAArrayAppend(self->timeSignRecords, timeSign);

    TempoRecord *tempo = malloc(sizeof(TempoRecord));
    *tempo = (TempoRecord){0, INT32_MAX, 120.0, 0, INT64_MAX};
    NAArrayAppend(self->tempoRecords, tempo);

    RepeatSection *section = malloc(sizeof(RepeatSection));
    *section = (RepeatSection){0, INT32_MAX};
    NAArrayAppend(self->repeatSections, section);

    return self;
}

void TimeTableDestroy(TimeTable *self)
{
    NAArrayTraverse(self->timeSignRecords, free);
    NAArrayTraverse(self->tempoRecords, free);
    NAArrayTraverse(self->repeatSections, free);
    NAArrayDestroy(self->timeSignRecords);
    NAArrayDestroy(self->tempoRecords);
    NAArrayDestroy(self->repeatSections);
    free(self);
}

bool TimeTableAddTimeSign(TimeTable *self, int32_t tick, TimeSign timeSign)
{
    Location l = TimeTableTick2Location(self, tick);
    if (1 != l.b || 0 != l.t) {
        return false;
    }

    int count = NAArrayCount(self->timeSignRecords);
    TimeSignRecord **records = NAArrayGetValues(self->timeSignRecords);

    int i = NAArrayBSearchIndex(self->timeSignRecords, &tick, TimeSignRecordFindByTickComparator);
    if (0 == memcmp(&records[i]->timeSign, &timeSign, sizeof(TimeSign))) {
        return true;
    }

    if (records[i]->tickStart == tick) {
        records[i]->timeSign = timeSign;
        goto REFRESH;
    }

    if (i + 1 < count) {
        if (0 == memcmp(&records[i + 1]->timeSign, &timeSign, sizeof(TimeSign))) {
            records[i + 1]->tickStart = tick;
            goto REFRESH;
        }
    }

    TimeSignRecord *insert = malloc(sizeof(TimeSignRecord));
    *insert = *records[i];
    records[i]->tickEnd = tick;
    insert->tickStart = tick;
    insert->timeSign = timeSign;

    NAArrayInsertAt(self->timeSignRecords, i + 1, insert);

REFRESH:
    TimeTableRefreshMeasureFrom(self, i);
    return true;
}

bool TimeTableAddTempo(TimeTable *self, int32_t tick, float tempo)
{
    int count = NAArrayCount(self->tempoRecords);
    TempoRecord **records = NAArrayGetValues(self->tempoRecords);

    int i = NAArrayBSearchIndex(self->tempoRecords, &tick, TempoRecordFindByTickComparator);
    if (records[i]->tempo == tempo) {
        return true;
    }

    if (records[i]->tickStart == tick) {
        records[i]->tempo = tempo;
        goto REFRESH;
    }

    if (i + 1 < count) {
        if (records[i + 1]->tempo == tempo) {
            records[i + 1]->tickStart = tick;
            goto REFRESH;
        }
    }

    TempoRecord *insert = malloc(sizeof(TempoRecord));
    *insert = *records[i];
    records[i]->tickEnd = tick;
    insert->tickStart = tick;
    insert->tempo = tempo;

    NAArrayInsertAt(self->tempoRecords, i + 1, insert);

REFRESH:
    TimeTableRefreshUsecFrom(self, i);
    return true;
}

extern bool TimeTableAddRepeatPoint(TimeTable *self, int32_t tick)
{
    RepeatSection **sections = NAArrayGetValues(self->repeatSections);

    int i = NAArrayBSearchIndex(self->repeatSections, &tick, RepeatSectionFindByTickComparator);
    if (sections[i]->tickStart == tick) {
        return true;
    }

    RepeatSection *insert = malloc(sizeof(TempoRecord));
    *insert = *sections[i];
    insert->tickEnd = sections[i]->tickEnd;
    sections[i]->tickEnd = tick;
    insert->tickStart = tick;

    NAArrayInsertAt(self->repeatSections, i + 1, insert);
    return true;
}

void TimeTableSetResolution(TimeTable *self, int32_t resolution)
{
    self->resolution = resolution;
    TimeTableRefreshMeasureFrom(self, 0);
}

void TimeTableSetLength(TimeTable *self, int32_t length)
{
    self->length = length;

    RepeatSection *section = NAArrayGetValueAt(self->repeatSections, NAArrayCount(self->repeatSections) - 1);
    section->tickEnd = length;
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
    TimeSignRecord **records = NAArrayGetValues(self->timeSignRecords);
    int i = NAArrayBSearchIndex(self->timeSignRecords, &measure, TimeSignRecordFindByMeasureComparator);

    return records[i]->tickStart + records[i]->measureLength * (measure - records[i]->measureStart);
}

int32_t TimeTableTickByLocation(TimeTable *self, Location location)
{
    TimeSignRecord **records = NAArrayGetValues(self->timeSignRecords);
    int i = NAArrayBSearchIndex(self->timeSignRecords, &location.m, TimeSignRecordFindByMeasureComparator);

    int32_t ret = records[i]->tickStart + records[i]->measureLength * (location.m - records[i]->measureStart);
    ret += (location.b - 1) * self->resolution * 4 / records[i]->timeSign.denominator;
    ret += location.t;
    return ret;
}

TimeSign TimeTableTimeSignOnTick(TimeTable *self, int32_t tick)
{
    TimeSignRecord **records = NAArrayGetValues(self->timeSignRecords);
    int i = NAArrayBSearchIndex(self->timeSignRecords, &tick, TimeSignRecordFindByTickComparator);
    return records[i]->timeSign;
}

float TimeTableTempoOnTick(TimeTable *self, int32_t tick)
{
    TempoRecord **records = NAArrayGetValues(self->tempoRecords);
    int i = NAArrayBSearchIndex(self->tempoRecords, &tick, TempoRecordFindByTickComparator);
    return records[i]->tempo;
}

Location TimeTableTick2Location(TimeTable *self, int32_t tick)
{
    TimeSignRecord **records = NAArrayGetValues(self->timeSignRecords);
    int i = NAArrayBSearchIndex(self->timeSignRecords, &tick, TimeSignRecordFindByTickComparator);

    int32_t tickFromPreviousTimeSign = (tick - records[i]->tickStart);
    int32_t tickPerBeat = self->resolution * 4 / records[i]->timeSign.denominator;

    Location ret;
    ret.m = records[i]->measureStart + tickFromPreviousTimeSign / records[i]->measureLength;
    ret.b = (tickFromPreviousTimeSign % records[i]->measureLength) / tickPerBeat + 1;
    ret.t = tick % tickPerBeat;

    return ret;
}

int64_t TimeTableTick2MicroSec(TimeTable *self, int32_t tick)
{
    TempoRecord **records = NAArrayGetValues(self->tempoRecords);
    int i = NAArrayBSearchIndex(self->tempoRecords, &tick, TempoRecordFindByTickComparator);

    double usecPerTick = 60 * 1000 * 1000 / records[i]->tempo / self->resolution;
    return round(records[i]->usecStart + (tick - records[i]->tickStart) * usecPerTick);
}

int32_t TimeTableMicroSec2Tick(TimeTable *self, int64_t usec)
{
    TempoRecord **records = NAArrayGetValues(self->tempoRecords);
    int i = NAArrayBSearchIndex(self->tempoRecords, &usec, TempoRecordFindByUsecComparator);

    double usecPerTick = 60 * 1000 * 1000 / records[i]->tempo / self->resolution;
    return round(records[i]->tickStart + (usec - records[i]->usecStart) / usecPerTick);
}

RepeatSection TimeTableRepeatSectionOnTick(TimeTable *self, int32_t tick)
{
    RepeatSection **sections = NAArrayGetValues(self->repeatSections);
    int i = NAArrayBSearchIndex(self->repeatSections, &tick, RepeatSectionFindByTickComparator);
    return *sections[i];
}

NAArray *TimeTableGetRepeatSections(TimeTable *self)
{
    return self->repeatSections;
}

static void TimeTableRefreshMeasureFrom(TimeTable *self, int index)
{
    int count = NAArrayCount(self->timeSignRecords);
    TimeSignRecord **records = NAArrayGetValues(self->timeSignRecords);

    TimeSignRecord *previous = 0 < index ? records[index - 1] : &((TimeSignRecord){ .measureEnd = 1 });
    for (int i = index; i < count; ++i) {
        records[i]->measureStart = previous->measureEnd;
        records[i]->measureLength = self->resolution * 4 / records[i]->timeSign.denominator * records[i]->timeSign.numerator;
        records[i]->measureEnd = records[i]->measureStart + (records[i]->tickEnd - records[i]->tickStart) / records[i]->measureLength;
        previous = records[i];
    }
}

static void TimeTableRefreshUsecFrom(TimeTable *self, int index)
{
    int count = NAArrayCount(self->tempoRecords);
    TempoRecord **records = NAArrayGetValues(self->tempoRecords);

    TempoRecord *previous = 0 < index ? records[index - 1] : &((TempoRecord){0});
    for (int i = index; i < count; ++i) {
        double usecPerTick = 60 * 1000 * 1000 / records[i]->tempo / self->resolution;
        records[i]->usecStart = previous->usecEnd;
        records[i]->usecEnd = records[i]->usecStart + round((records[i]->tickEnd - records[i]->tickStart) * usecPerTick);
        previous = records[i];
    }
}

static int TimeSignRecordFindByTickComparator(const void *_tick, const void *_record)
{
    const int32_t *tick = _tick;
    const TimeSignRecord *record = *((const TimeSignRecord **)_record);

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
    const TimeSignRecord *record = *((const TimeSignRecord **)_record);

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
    const TempoRecord *record = *((const TempoRecord **)_record);

    if (record->tickStart <= *tick && *tick < record->tickEnd) {
        return 0;
    }
    else {
        return *tick - record->tickStart;
    }
}

static int TempoRecordFindByUsecComparator(const void *_usec, const void *_record)
{
    const int64_t *usec = _usec;
    const TempoRecord *record = *((const TempoRecord **)_record);

    if (record->usecStart <= *usec && *usec < record->usecEnd) {
        return 0;
    }
    else {
        return *usec - record->usecStart;
    }
}

static int RepeatSectionFindByTickComparator(const void *_tick, const void *_section)
{
    const int32_t *tick = _tick;
    const RepeatSection *section = *((const RepeatSection **)_section);

    if (section->tickStart <= *tick && *tick < section->tickEnd) {
        return 0;
    }
    else {
        return *tick - section->tickStart;
    }
}

void TimeSignRecordDump(TimeSignRecord *self, int indent)
{
    printf("%*s", indent, "");
    printf("tickStart=%d tickEnd=%d timeSign=%d/%d measureStart=%d measureEnd=%d measureLength=%d\n",
            self->tickStart, self->tickEnd, self->timeSign.numerator, self->timeSign.denominator, self->measureStart, self->measureEnd, self->measureLength);
}

void TempoRecordDump(TempoRecord *self, int indent)
{
    printf("%*s", indent, "");
    printf("tickStart=%d tickEnd=%d tempo=%f usecStart=%lld usecEnd=%lld\n",
            self->tickStart, self->tickEnd, self->tempo, self->usecStart, self->usecEnd);
}

void RepeatSectionDump(RepeatSection *self, int indent)
{
    printf("%*s", indent, "");
    printf("tickStart=%d tickEnd=%d\n", self->tickStart, self->tickEnd);
}

void TimeTableDump(TimeTable *self, int indent)
{
    NAIterator *iterator;

    printf("\n");
    printf("%*s", indent, "");
    printf("TimeTable: resolution=%d length=%d\n", self->resolution, self->length);
    printf("%*s", indent, "");
    printf("-------------------------\n");
    printf("\n");
    printf("%*s", indent, "");
    printf("TimeSign:\n");
    printf("%*s", indent, "");
    printf("-------------------------\n");
    iterator = NAArrayGetIterator(self->timeSignRecords);
    while (iterator->hasNext(iterator)) {
        TimeSignRecordDump(iterator->next(iterator), indent);
    }
    printf("\n");
    printf("%*s", indent, "");
    printf("Tempo:\n");
    printf("%*s", indent, "");
    printf("-------------------------\n");
    iterator = NAArrayGetIterator(self->tempoRecords);
    while (iterator->hasNext(iterator)) {
        TempoRecordDump(iterator->next(iterator), indent);
    }
    printf("\n");
    printf("%*s", indent, "");
    printf("RepeatSection:\n");
    printf("%*s", indent, "");
    printf("-------------------------\n");
    iterator = NAArrayGetIterator(self->repeatSections);
    while (iterator->hasNext(iterator)) {
        RepeatSectionDump(iterator->next(iterator), indent);
    }
}
