#pragma once

#include <stdbool.h>

extern void *NATimeMeasureStart(const char *file, const char *function, int line);
extern void NATimeMeasure(void *self, const char *file, const char *function, int line);
extern void NATimeMeasureFinish(void *self, const char *file, const char *function, int line);

extern void NATimeWaitUntil(void *context, int msec, int count, bool (*until)(void *context));

#define NATimeMeasureStart() NATimeMeasureStart(__FILE__, __func__, __LINE__)
#define NATimeMeasure(self) NATimeMeasure(self, __FILE__, __func__, __LINE__)
#define NATimeMeasureFinish(self) NATimeMeasureFinish(self, __FILE__, __func__, __LINE__)
