#include "NAArray.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#undef NAArrayGetValues
#undef NAArrayTraverse
#undef NAArrayTraverseWithContext
#undef NAArrayApplyAt

struct _NAArray {
    int capacity;
    int count;
    intptr_t *values;
    NADescription description;
};

typedef struct _NAArrayIterator {
    NAIterator super;
    NAArray *array;
    int index;
} NAArrayIterator;

const int NAArrayIteratorSize = sizeof(NAArrayIterator);

static intptr_t *NAArrayMemmovePointer(intptr_t *dst, const intptr_t *src, int count)
{
	if (dst <= src) {
        while (count--)
            *dst++ = *src++;
    }
    else {
        src += count;
        dst += count;

        while (count--)
            *--dst = *--src;
    }

	return dst;
}

NAArray *NAArrayCreate(int initialCapacity, NADescription description)
{
    NAArray *self = calloc(1, sizeof(NAArray));
    self->capacity = initialCapacity;
    self->values = malloc(initialCapacity * sizeof(intptr_t));
    self->description = description ? description : NADescriptionAddress;
    return self;
}

void NAArrayDestroy(NAArray *self)
{
    free(self->values);
    free(self);
}

int NAArrayCount(NAArray *self)
{
    return self->count;
}

void **NAArrayGetValues(NAArray *self)
{
    return (void **)self->values;
}

void *NAArrayGetValueAt(NAArray *self, int index)
{
    return (void *)self->values[index];
}

void NAArrayAppend(NAArray *self, void *value)
{
    if (self->capacity <= self->count + 1) {
        self->capacity *= 2;
        self->values = realloc(self->values, self->capacity * sizeof(intptr_t));
    }

    self->values[self->count++] = (intptr_t)value;
}

bool NAArrayInsertAt(NAArray *self, int index, void *value)
{
    if (index < 0 || self->count < index) {
        return false;
    }

    if (self->capacity <= self->count + 1) {
        self->capacity *= 2;
        self->values = realloc(self->values, self->capacity * sizeof(intptr_t));
    }


    NAArrayMemmovePointer(self->values + index + 1, self->values + index, self->count++ - index);
    return true;
}

bool NAArrayRemoveAt(NAArray *self, int index)
{
    if (self->count <= index) {
        return false;
    }

    NAArrayMemmovePointer(self->values + index , self->values + index + 1, --self->count - index);
    return true;
}

void NAArraySort(NAArray *self, int (*comparator)(const void *, const void *))
{
    qsort(self->values, self->count, sizeof(intptr_t), comparator);
}

int NAArrayFindFirstIndex(NAArray *self, const void *key, int (*comparator)(const void *, const void *))
{
    for (int i = 0; i < self->count; ++i) {
        if (0 == comparator(key, (const void *)self->values[i])) {
            return i;
        }
    }

    return -1;
}

int NAArrayFindLastIndex(NAArray *self, const void *key, int (*comparator)(const void *, const void *))
{
    for (int i = self->count - 1; 0 <= i; --i) {
        if (0 == comparator(key, (const void *)self->values[i])) {
            return i;
        }
    }

    return -1;
}

int NAArrayBSearchIndex(NAArray *self, const void *key, int (*comparator)(const void *, const void *))
{
    intptr_t *result = bsearch(key, self->values, self->count, sizeof(intptr_t), comparator);
    return NULL != result ? (int)(result - self->values) : -1;
}

void NAArrayTraverse(NAArray *self, void (*function)(void *))
{
    for (int i = 0; i < self->count; ++i) {
        function((void *)self->values[i]);
    }
}

void NAArrayTraverseWithContext(NAArray *self, void *context, void (*function)(void *, void *, va_list), ...)
{
    va_list argList;
    va_start(argList, function);

    for (int i = 0; i < self->count; ++i) {
        function(context, (void *)self->values[i], argList);
    }

    va_end(argList);
}

void NAArrayApplyAt(NAArray *self, int index, void (*function)(void *))
{
    function((void *)self->values[index]);
}

static bool NAArrayIteratorHasNext(NAIterator *_iterator)
{
    NAArrayIterator *iterator = (NAArrayIterator *)_iterator;
    return iterator->index < iterator->array->count;
}

static void *NAArrayIteratorNext(NAIterator *_iterator)
{
    NAArrayIterator *iterator = (NAArrayIterator *)_iterator;
    return (void *)iterator->array->values[iterator->index++];
}

NAIterator *NAArrayGetIterator(NAArray *self, void *buffer)
{
    NAArrayIterator *iterator = buffer;
    iterator->super.hasNext = NAArrayIteratorHasNext;
    iterator->super.next = NAArrayIteratorNext;
    iterator->array = self;
    iterator->index = 0;

    return (NAIterator *)iterator;
}

void NAArrayDescription(void *_self, FILE *stream)
{
    NAArray *self = _self;
    fprintf(stream, "<NAArray capacity=%d count=%d 0x%X\n", self->capacity, self->count, (uint32_t)self);

    for (int i = 0; i < self->count; ++i) {
        fprintf(stream, "  [%d] ", i);
        self->description((void *)self->values[i], stream);
        fprintf(stream, "\n");
    }

    fprintf(stream, ">\n");
}
