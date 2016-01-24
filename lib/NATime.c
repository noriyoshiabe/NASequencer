#include "NATime.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#undef NATimeMeasureStart
#undef NATimeMeasure
#undef NATimeMeasureFinish

#define MAX_MEASURE_POINT 32

typedef struct TimeMeasureContext {
    struct {
        const char *file;
        const char *function;
        int line;
        int64_t msec;
    } points[MAX_MEASURE_POINT];

    int count;
} TimeMeasureContext;

static int64_t currentMicroSec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}

void *NATimeMeasureStart(const char *file, const char *function, int line)
{
    TimeMeasureContext *self = calloc(1, sizeof(TimeMeasureContext));
    self->points[self->count].msec = currentMicroSec();
    self->points[self->count].file = file;
    self->points[self->count].function = function;
    self->points[self->count].line = line;
    self->count = 1;
    return self;
}

void NATimeMeasure(void *_self, const char *file, const char *function, int line)
{
    TimeMeasureContext *self = _self;
    if (MAX_MEASURE_POINT <= self->count) {
        return;
    }

    self->points[self->count].msec = currentMicroSec();
    self->points[self->count].file = file;
    self->points[self->count].function = function;
    self->points[self->count].line = line;
    ++self->count;
}

void NATimeMeasureFinish(void *_self, const char *file, const char *function, int line)
{
    TimeMeasureContext *self = _self;

    if (self->count < MAX_MEASURE_POINT) {
        NATimeMeasure(self, file, function, line);
    }

    int64_t msec = self->points[0].msec;

    printf("---------------------------------------\n");

    for (int i = 0; i < self->count; ++i) {
        printf("[msec: %lld, delta: %lld] %s:%s - %d\n",
                self->points[i].msec,
                self->points[i].msec - msec,
                self->points[i].file,
                self->points[i].function,
                self->points[i].line);

        msec = self->points[i].msec;
    }

    printf("---------------------------------------\n");

    free(self);
}

void NATimeWaitUntil(void *context, int msec, int count, bool (*until)(void *context))
{
    for (int i = 0; i < count; ++i) {
        if (until(context)) {
            break;
        }
        else {
            usleep(msec);
        }
    }
}
