#pragma once

#include <stdbool.h>
#include <stdarg.h>

#include "NADescription.h"
#include "NAIterator.h"
#include "alloca.h"

typedef struct _NAArray NAArray;

extern NAArray *NAArrayCreate(int initialCapacity, NADescription description);
extern void NAArrayDestroy(NAArray *self);
extern int NAArrayCount(NAArray *self);
extern bool NAArrayIsEmpty(NAArray *self);
extern void **NAArrayGetValues(NAArray *self);
extern void *NAArrayGetValueAt(NAArray *self, int index);
extern void NAArrayAppend(NAArray *self, void *value);
extern void NAArrayAppendAll(NAArray *self, NAArray *array);
extern bool NAArrayInsertAt(NAArray *self, int index, void *value);
extern bool NAArrayRemoveAt(NAArray *self, int index);
extern void NAArrayRemoveAll(NAArray *self);
extern void NAArraySort(NAArray *self, int (*comparator)(const void *, const void *));
extern int NAArrayFindFirstIndex(NAArray *self, const void *key, int (*comparator)(const void *, const void *));
extern int NAArrayFindLastIndex(NAArray *self, const void *key, int (*comparator)(const void *, const void *));
extern int NAArrayBSearchIndex(NAArray *self, const void *key, int (*comparator)(const void *, const void *));
extern void NAArrayTraverse(NAArray *self, void (*function)(void *));
extern void NAArrayTraverseWithContext(NAArray *self, void *context, void (*function)(void *, void *, va_list), ...);
extern void NAArrayApplyAt(NAArray *self, int index, void (*function)(void *));
extern NAIterator *NAArrayGetIterator(NAArray *self, void *buffer);
extern void NAArrayDescription(void *self, FILE *stream);

extern int NAArrayAddressComparator(const void *p1, const void *p2);
extern const int NAArrayIteratorSize;

#define NAArrayGetValues(self) (void *)NAArrayGetValues(self)
#define NAArrayTraverse(self, function) NAArrayTraverse(self, (void *)function)
#define NAArrayTraverseWithContext(self, context, function, ...) NAArrayTraverseWithContext(self, context, (void *)function, __VA_ARGS__)
#define NAArrayApplyAt(self, index, function) NAArrayApplyAt(self, index, (void *)function)
#define NAArrayGetIterator(self) NAArrayGetIterator(self, alloca(NAArrayIteratorSize))
