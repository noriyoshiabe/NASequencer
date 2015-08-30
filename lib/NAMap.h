#pragma once

#include "NAHash.h"
#include "NADescription.h"
#include "NAIterator.h"

#include <stdbool.h>

typedef struct _NAMap NAMap;

typedef struct _NAMapEntry {
    void *key;
    void *value;
} NAMapEntry;

extern NAMap *NAMapCreate(NAHash keyHash, NADescription keyDescription, NADescription valueDescription);
extern void NAMapDestroy(NAMap *self);
extern bool NAMapContainsKey(NAMap *self, void *key);
extern void *NAMapPut(NAMap *self, void *key, void *value);
extern void *NAMapGet(NAMap *self, void *key);
extern int NAMapCount(NAMap *self);
extern void *NAMapRemove(NAMap *self, void *key);
extern void NAMapTraverseKey(NAMap *self, void (*function)(void *));
extern void NAMapTraverseValue(NAMap *self, void (*function)(void *));
extern NAIterator *NAMapGetIterator(NAMap *self, void *buffer);
extern void NAMapDescription(void *self, FILE *stream);

extern const int NAMapIteratorSize;
