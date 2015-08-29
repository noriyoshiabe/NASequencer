#include "NAMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Entry {
    void *key;
    void *value;
    struct _Entry *next;
} Entry;

struct _NAMap {
    Entry *entries;
    Entry **buckets;
    Entry *freeList;
    int count;
    int size;
    NAHash keyHash;
    NADescription keyDescription;
    NADescription valueDescription;
};

typedef struct _NAMapIterator {
    NAIterator super;
    NAMap *map;
    int bucketIndex;
    Entry *entry;
} NAMapIterator;

const int NAMapIteratorSize = sizeof(NAMapIterator);

NAMap *NAMapCreate(NAHash keyHash, NADescription keyDescription, NADescription valueDescription)
{
    const int __initialSize = 32;
    
    NAMap *self = (NAMap *)malloc(sizeof(NAMap));
    self->entries = (Entry *)calloc(__initialSize, sizeof(Entry));
    self->buckets = (Entry **)calloc(__initialSize, sizeof(Entry *));
    self->size = __initialSize;
    self->count = 0;
    self->keyHash = keyHash ? keyHash : NAHashAddress;
    self->keyDescription = keyDescription ? keyDescription : NADescriptionAddress;
    self->valueDescription = valueDescription ? valueDescription : NADescriptionAddress;

    self->freeList = self->entries;
    for (int i = 0; i < self->size; ++i) {
        self->entries[i].next = i < self->size - 1 ? &self->entries[i + 1] : NULL;
    }

    return self;
}

void NAMapDestroy(NAMap *self)
{
    free(self->entries);
    free(self->buckets);
    free(self);
}

bool NAMapContainsKey(NAMap *self, void *key)
{
    uint32_t h = self->keyHash(key);

    Entry *entry = self->buckets[h & (self->size - 1)];
    while (entry != NULL) {
        if (h == self->keyHash(entry->key)) {
            return true;
        }
        entry = entry->next;
    }

    return false;
}

static void __NAMapAdd(NAMap *self, void *key, void *value)
{
    uint32_t h = self->keyHash(key);
    int idx = h & (self->size - 1);

    Entry *entry = self->freeList;
    self->freeList = self->freeList->next;

    entry->next = self->buckets[idx] ? self->buckets[idx] : NULL;
    self->buckets[idx] = entry;

    entry->key = key;
    entry->value = value;

    ++self->count;
}

static void __NAMapExtend(NAMap *self)
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
            __NAMapAdd(self, entry->key, entry->value);
            entry = entry->next;
        }
    }

    free(oldEntries);
    free(oldBuckets);
}

void *NAMapPut(NAMap *self, void *key, void *value)
{
    if (NAMapContainsKey(self, key)) {
        NAMapRemove(self, key);
    }

    if (self->count >= self->size) {
        __NAMapExtend(self);
    }

    __NAMapAdd(self, key, value);

    return value;
}

void *NAMapGet(NAMap *self, void *key)
{
    uint32_t h = self->keyHash(key);

    Entry *entry = self->buckets[h & (self->size - 1)];
    while (entry != NULL) {
        if (h == self->keyHash(entry->key)) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

int NAMapCount(NAMap *self)
{
    return self->count;
}

void *NAMapRemove(NAMap *self, void *key)
{
    uint32_t h = self->keyHash(key);
    int idx = h & (self->size - 1);

    Entry *entry = self->buckets[idx];
    Entry *prev = NULL;
    while (entry != NULL) {
        if (h == self->keyHash(entry->key)) {
            if (prev) {
                prev->next = entry->next;
            }
            else {
                self->buckets[idx] = NULL;
            }

            entry->next = self->freeList;
            self->freeList = entry;
            --self->count;

            return entry->value;
        }

        prev = entry;
        entry = entry->next;
    }

    return NULL;
}

static bool NAMapIteratorHasNext(NAIterator *_iterator)
{
    NAMapIterator *iterator = (NAMapIterator *)_iterator;
    return NULL != iterator->entry;
}

static void *NAMapIteratorNext(NAIterator *_iterator)
{
    NAMapIterator *iterator = (NAMapIterator *)_iterator;
    void *ret = iterator->entry;

    iterator->entry = iterator->entry->next;
    if (!iterator->entry) {
        for (iterator->bucketIndex += 1; iterator->bucketIndex < iterator->map->size; ++iterator->bucketIndex) {
            iterator->entry = iterator->map->buckets[iterator->bucketIndex];
            if (iterator->entry) {
                break;
            }
        }
    }

    return ret;
}

NAIterator *NAMapGetIterator(NAMap *self, void *buffer)
{
    NAMapIterator *iterator = buffer;
    iterator->super.hasNext = NAMapIteratorHasNext;
    iterator->super.next = NAMapIteratorNext;
    iterator->map = self;

    for (iterator->bucketIndex = 0; iterator->bucketIndex < self->size; ++iterator->bucketIndex) {
        iterator->entry = self->buckets[iterator->bucketIndex];
        if (iterator->entry) {
            break;
        }
    }

    return (NAIterator *)iterator;
}

void NAMapDescription(void *_self, FILE *stream)
{
    NAMap *self = _self;
    fprintf(stream, "<NAMap 0x%X\n", (uint32_t)self);

    for (int i = 0; i < self->size; ++i) {
        Entry *entry = self->buckets[i];
        int chain = 0;
        while (entry != NULL) {
            fprintf(stream, "  [%d:%d] ", i, chain);
            self->keyDescription(entry->key, stream);
            fprintf(stream, ": ");
            self->valueDescription(entry->value, stream);
            fprintf(stream, "\n");
            entry = entry->next;
            ++chain;
        }
    }

    fprintf(stream, ">\n");
}
