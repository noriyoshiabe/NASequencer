#include "NAArray.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct _NAArray {
    int capacity;
    int elementSize;
    int count;
    uint8_t *bytes;
    NADescription description;
    void *(*__memcpy__)(void *dst, const void *src, size_t size);
    void *(*__memmove__)(void *dst, const void *src, size_t size);
};

typedef struct _NAArrayIterator {
    NAIterator super;
    NAArray *array;
    int index;
    int byteCount;
} NAArrayIterator;

const int NAArrayIteratorSize = sizeof(NAArrayIterator);

static void *NAArrayMemcpy16(void *_dst, const void *_src, size_t size)
{
	const int16_t *src = _src;
	int16_t *dst = _dst;
    int count = size >> 1;

	while (count--)
		*dst++ = *src++;

	return _dst;
}

static void *NAArrayMemcpy32(void *_dst, const void *_src, size_t size)
{
	const int32_t *src = _src;
	int32_t *dst = _dst;
    int count = size >> 2;

	while (count--)
		*dst++ = *src++;

	return _dst;
}

static void *NAArrayMemcpy64(void *_dst, const void *_src, size_t size)
{
	const int64_t *src = _src;
	int64_t *dst = _dst;
    int count = size >> 3;

	while (count--)
		*dst++ = *src++;

	return _dst;
}

static void *NAArrayMemmove16(void *_dst, const void *_src, size_t size)
{
	const int16_t *src = _src;
	int16_t *dst = _dst;
    int count = size >> 1;

	if (_dst <= _src) {
        while (count--)
            *dst++ = *src++;
    }
    else {
        src += count;
        dst += count;

        while (count--)
            *--dst = *--src;
    }

	return _dst;
}

static void *NAArrayMemmove32(void *_dst, const void *_src, size_t size)
{
	const int32_t *src = _src;
	int32_t *dst = _dst;
    int count = size >> 2;

	if (_dst <= _src) {
        while (count--)
            *dst++ = *src++;
    }
    else {
        src += count;
        dst += count;

        while (count--)
            *--dst = *--src;
    }

	return _dst;
}

static void *NAArrayMemmove64(void *_dst, const void *_src, size_t size)
{
	const int64_t *src = _src;
	int64_t *dst = _dst;
    int count = size >> 3;

	if (_dst <= _src) {
        while (count--)
            *dst++ = *src++;
    }
    else {
        src += count;
        dst += count;

        while (count--)
            *--dst = *--src;
    }

	return _dst;
}

NAArray *NAArrayCreate(int capacity, int elementSize, NADescription description)
{
    NAArray *self = calloc(1, sizeof(NAArray));
    self->capacity = capacity;
    self->elementSize = elementSize;
    self->bytes = malloc(capacity * elementSize);
    self->description = description ? description : NADescriptionAddress;
    self->__memcpy__ =
        0 == elementSize % sizeof(int64_t) ? NAArrayMemcpy64 : 
        0 == elementSize % sizeof(int32_t) ? NAArrayMemcpy32 : 
        0 == elementSize % sizeof(int16_t) ? NAArrayMemcpy16 : memcpy;
    self->__memmove__ =
        0 == elementSize % sizeof(int64_t) ? NAArrayMemmove64 : 
        0 == elementSize % sizeof(int32_t) ? NAArrayMemmove32 : 
        0 == elementSize % sizeof(int16_t) ? NAArrayMemmove16 : memmove;
    return self;
}

void NAArrayDestroy(NAArray *self)
{
    free(self->bytes);
    free(self);
}

int NAArrayCount(NAArray *self)
{
    return self->count;
}

void *NAArrayGetValues(NAArray *self)
{
    return self->bytes;
}

void NAArrayAppend(NAArray *self, void *value)
{
    if (self->capacity <= self->count + 1) {
        self->capacity *= 2;
        self->bytes = realloc(self->bytes, self->capacity * self->elementSize);
    }

    self->__memcpy__(self->bytes + self->elementSize * self->count++, value, self->elementSize);
}

bool NAArrayInsertAt(NAArray *self, int index, void *value)
{
    if (index < 0 || self->count < index) {
        return false;
    }

    if (self->capacity <= self->count + 1) {
        self->capacity *= 2;
        self->bytes = realloc(self->bytes, self->capacity * self->elementSize);
    }

    self->__memmove__(
            self->bytes + self->elementSize * (index + 1),
            self->bytes + self->elementSize * index,
            self->elementSize * (self->count++ - index));

    return true;
}

bool NAArrayRemoveAtIndex(NAArray *self, int index)
{
    if (self->count <= index) {
        return false;
    }

    self->__memmove__(
            self->bytes + self->elementSize * index,
            self->bytes + self->elementSize * (index + 1),
            self->elementSize * (--self->count - index));

    return true;
}

void NAArraySort(NAArray *self, int (*comparator)(const void *, const void *))
{
    qsort(self->bytes, self->count, self->elementSize, comparator);
}

int NAArrayFindIndex(NAArray *self, const void *key, int (*comparator)(const void *, const void *))
{
    uint8_t *result = bsearch(key, self->bytes, self->count, self->elementSize, comparator);
    return NULL != result ? (int)(result - self->bytes) / self->elementSize : -1;
}

void NAArrayTraverse(NAArray *self, void (*function)(void *))
{
    for (int i = 0; i < self->count; ++i) {
        void *value = self->bytes + i * self->elementSize;
        function(value);
    }
}

void NAArrayTraverseWithArg(NAArray *self, void (*function)(void *, void *), void *arg)
{
    for (int i = 0; i < self->count; ++i) {
        void *value = self->bytes + i * self->elementSize;
        function(value, arg);
    }
}

void NAArrayTraverseWithArgList(NAArray *self, void (*function)(void *, va_list), ...)
{
    va_list argList;
    va_start(argList, function);

    for (int i = 0; i < self->count; ++i) {
        void *value = self->bytes + i * self->elementSize;
        function(value, argList);
    }

    va_end(argList);
}

static bool NAArrayIteratorHasNext(NAIterator *_iterator)
{
    NAArrayIterator *iterator = (NAArrayIterator *)_iterator;
    return iterator->index < iterator->byteCount;
}

static void *NAArrayIteratorNext(NAIterator *_iterator)
{
    NAArrayIterator *iterator = (NAArrayIterator *)_iterator;
    void *ret = iterator->array->bytes + iterator->index;
    iterator->index += iterator->array->elementSize;
    return ret;
}

NAIterator *NAArrayGetIterator(NAArray *self, void *buffer)
{
    NAArrayIterator *iterator = buffer;
    iterator->super.hasNext = NAArrayIteratorHasNext;
    iterator->super.next = NAArrayIteratorNext;
    iterator->array = self;
    iterator->index = 0;
    iterator->byteCount = self->count * self->elementSize;

    return (NAIterator *)iterator;
}

void NAArrayDump(NAArray *self)
{
    printf("<NAArray capacity=%d count=%d 0x%X\n", self->capacity, self->count, (uint32_t)self);

    for (int i = 0; i < self->count; ++i) {
        void *value = self->bytes + i * self->elementSize;
        char buf[1024];
        self->description(value, buf, 1024);
        printf("  [%d] %s\n", i, buf);
    }

    printf(">\n");
}
