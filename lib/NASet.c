#include "NASet.h"
#include "NAHash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef NASetTraverse
#undef NASetGetIterator

typedef struct _Entry {
    void *value;
    struct _Entry *next;
} Entry;

struct _NASet {
    Entry *entries;
    Entry **buckets;
    Entry *freeList;
    int count;
    int size;
    NAHash hash;
    NADescription description;
};

typedef struct _NASetIterator {
    NAIterator super;
    NASet *set;
    int bucketIndex;
    Entry *current;
    Entry *next;
} NASetIterator;

const int NASetIteratorSize = sizeof(NASetIterator);

NASet *NASetCreate(NAHash hash, NADescription description)
{
    const int __initialSize = 32;

    NASet *self = (NASet *)malloc(sizeof(NASet));
    self->entries = (Entry *)calloc(__initialSize, sizeof(Entry));
    self->buckets = (Entry **)calloc(__initialSize, sizeof(Entry *));
    self->size = __initialSize;
    self->count = 0;
    self->hash = hash ? hash : NAHashAddress;
    self->description = description ? description : NADescriptionAddress;

    self->freeList = self->entries;
    for (int i = 0; i < self->size; ++i) {
        self->entries[i].next = i < self->size - 1 ? &self->entries[i + 1] : NULL;
    }

    return self;
}

void NASetDestroy(NASet *self)
{
    free(self->entries);
    free(self->buckets);
    free(self);
}

bool NASetContains(NASet *self, void *value)
{
    uint32_t h = self->hash(value);

    Entry *entry = self->buckets[h & (self->size - 1)];
    while (entry != NULL) {
        if (h == self->hash(entry->value)) {
            return true;
        }
        entry = entry->next;
    }

    return false;
}

static void __NASetAdd(NASet *self, void *value)
{
    uint32_t h = self->hash(value);
    int idx = h & (self->size - 1);

    Entry *entry = self->freeList;
    self->freeList = self->freeList->next;

    entry->next = self->buckets[idx] ? self->buckets[idx] : NULL;
    self->buckets[idx] = entry;

    entry->value = value;

    ++self->count;
}

static void __NASetExtend(NASet *self)
{
    Entry *oldEntries = self->entries;
    Entry **oldBuckets = self->buckets;
    int oldSize = self->size;

    self->size *= 2;
    self->entries = (Entry *)calloc(self->size, sizeof(Entry));
    self->buckets = (Entry **)calloc(self->size, sizeof(Entry *));
    self->count = 0;

    self->freeList = self->entries;
    for (int i = 0; i < self->size; ++i) {
        self->entries[i].next = i < self->size - 1 ? &self->entries[i + 1] : NULL;
    }

    for (int i = 0; i < oldSize; ++i) {
        Entry *entry = oldBuckets[i];
        while (NULL != entry) {
            __NASetAdd(self, entry->value);
            entry = entry->next;
        }
    }

    free(oldEntries);
    free(oldBuckets);
}

bool NASetAdd(NASet *self, void *value)
{
    if (NASetContains(self, value)) {
        return false;
    }

    if (self->count >= self->size) {
        __NASetExtend(self);
    }

    __NASetAdd(self, value);
    return true;
}

void *NASetGet(NASet *self, void *value)
{
    uint32_t h = self->hash(value);

    Entry *entry = self->buckets[h & (self->size - 1)];
    while (entry != NULL) {
        if (h == self->hash(entry->value)) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

int NASetCount(NASet *self)
{
    return self->count;
}

bool NASetRemove(NASet *self, void *value)
{
    uint32_t h = self->hash(value);
    int idx = h & (self->size - 1);

    Entry *entry = self->buckets[idx];
    Entry *prev = NULL;
    while (entry != NULL) {
        if (h == self->hash(entry->value)) {
            if (prev) {
                prev->next = entry->next;
            }
            else {
                self->buckets[idx] = NULL;
            }

            entry->next = self->freeList;
            self->freeList = entry;
            --self->count;

            return true;
        }

        prev = entry;
        entry = entry->next;
    }

    return false;
}

void NASetTraverse(NASet *self, void (*function)(void *))
{
    for (int i = 0; i < self->size; ++i) {
        Entry *entry = self->buckets[i];
        while (entry != NULL) {
            function(entry->value);
            entry = entry->next;
        }
    }
}

static bool NASetIteratorHasNext(NAIterator *_iterator)
{
    NASetIterator *iterator = (NASetIterator *)_iterator;
    return NULL != iterator->next;
}

static void *NASetIteratorNext(NAIterator *_iterator)
{
    NASetIterator *iterator = (NASetIterator *)_iterator;
    iterator->current = iterator->next;

    iterator->next = iterator->next->next;
    if (!iterator->next) {
        for (iterator->bucketIndex += 1; iterator->bucketIndex < iterator->set->size; ++iterator->bucketIndex) {
            iterator->next = iterator->set->buckets[iterator->bucketIndex];
            if (iterator->next) {
                break;
            }
        }
    }

    return iterator->current->value;
}

static void NASetIteratorRemove(NAIterator *_iterator)
{
    NASetIterator *iterator = (NASetIterator *)_iterator;
    if (iterator->current) {
        NASetRemove(iterator->set, iterator->current->value);
    }
}

NAIterator *NASetGetIterator(NASet *self, void *buffer)
{
    NASetIterator *iterator = buffer;
    iterator->super.hasNext = NASetIteratorHasNext;
    iterator->super.next = NASetIteratorNext;
    iterator->super.remove = NASetIteratorRemove;
    iterator->set = self;
    iterator->current = NULL;

    for (iterator->bucketIndex = 0; iterator->bucketIndex < self->size; ++iterator->bucketIndex) {
        iterator->next = self->buckets[iterator->bucketIndex];
        if (iterator->next) {
            break;
        }
    }

    return (NAIterator *)iterator;
}

void NASetDescription(void *_self, FILE *stream)
{
    NASet *self = _self;
    fprintf(stream, "<NASet 0x%X\n", (uint32_t)self);

    for (int i = 0; i < self->size; ++i) {
        Entry *entry = self->buckets[i];
        int chain = 0;
        while (entry != NULL) {
            fprintf(stream, "  [%d:%d] ", i, chain);
            self->description(entry->value, stream);
            fprintf(stream, "\n");
            entry = entry->next;
            ++chain;
        }
    }

    fprintf(stream, ">\n");
}
