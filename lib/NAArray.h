#pragma once

#include <stdbool.h>

typedef struct _NAArray NAArray;

extern NAArray *NAArrayCreate(int capacity, int elementSize);
extern void NAArrayDestroy(NAArray *self);
extern int NAArrayCount(NAArray *self);
extern void *NAArrayGetValues(NAArray *self);
extern void NAArrayAppend(NAArray *self, void *value);
extern bool NAArrayRemoveAtIndex(NAArray *self, int index);
