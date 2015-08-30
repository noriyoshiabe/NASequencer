#pragma once

#include <stdbool.h>
#include <stdarg.h>

#include "NADescription.h"
#include "NAIterator.h"

typedef struct _NAArray NAArray;

extern NAArray *NAArrayCreate(int initialCapacity, NADescription description);
extern void NAArrayDestroy(NAArray *self);
extern int NAArrayCount(NAArray *self);
extern void *NAArrayGetValues(NAArray *self);
extern void NAArrayAppend(NAArray *self, void *value);
extern bool NAArrayInsertAt(NAArray *self, int index, void *value);
extern bool NAArrayRemoveAtIndex(NAArray *self, int index);
extern void NAArraySort(NAArray *self, int (*comparator)(const void *, const void *));
extern int NAArrayFindIndex(NAArray *self, const void *key, int (*comparator)(const void *, const void *));
extern void NAArrayTraverse(NAArray *self, void (*function)(void *));
extern NAIterator *NAArrayGetIterator(NAArray *self, void *buffer);
extern void NAArrayDescription(void *self, FILE *stream);

extern const int NAArrayIteratorSize;
