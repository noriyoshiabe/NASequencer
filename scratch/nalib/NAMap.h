#pragma once

#include "NAHash.h"
#include "NADescription.h"

#include <stdbool.h>

typedef struct _NAMap NAMap;

extern NAMap *NAMapCreate(NAHash keyHash, NADescription keyDescription, NADescription valueDescription);
extern void NAMapDestroy(NAMap *self);
extern bool NAMapContainsKey(NAMap *self, void *key);
extern void *NAMapPut(NAMap *self, void *key, void *value);
extern void *NAMapGet(NAMap *self, void *key);
extern int NAMapCount(NAMap *self);
extern void *NAMapRemove(NAMap *self, void *key);
extern void NAMapDump(NAMap *self);
