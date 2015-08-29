#pragma once

#include <stdbool.h>
#include <stdarg.h>

typedef struct _NAArray NAArray;

extern NAArray *NAArrayCreate(int capacity, int elementSize);
extern void NAArrayDestroy(NAArray *self);
extern int NAArrayCount(NAArray *self);
extern void *NAArrayGetValues(NAArray *self);
extern void NAArrayAppend(NAArray *self, void *value);
extern bool NAArrayInsertAt(NAArray *self, int index, void *value);
extern bool NAArrayRemoveAtIndex(NAArray *self, int index);
extern void NAArraySort(NAArray *self, int (*comparator)(const void *, const void *));
extern int NAArrayFindIndex(NAArray *self, const void *key, int (*comparator)(const void *, const void *));
extern void NAArrayTraverse(NAArray *self, void (*function)(void *));
extern void NAArrayTraverseWithArg(NAArray *self, void (*function)(void *, void *), void *arg);
extern void NAArrayTraverseWithArgList(NAArray *self, void (*function)(void *, va_list), ...);
