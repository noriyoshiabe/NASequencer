#pragma once

#include "NAHash.h"
#include "NADescription.h"
#include "NAIterator.h"

#include <stdbool.h>

typedef struct _NASet NASet;

extern NASet *NASetCreate(NAHash hash, NADescription description);
extern void NASetDestroy(NASet *self);
extern bool NASetContains(NASet *self, void *value);
extern bool NASetAdd(NASet *self, void *value);
extern void *NASetGet(NASet *self, void *value);
extern int NASetCount(NASet *self);
extern bool NASetRemove(NASet *self, void *value);
extern NAIterator *NASetGetIterator(NASet *self, void *buffer);
extern void NASetDescription(void *self, FILE *stream);

extern const int NASetIteratorSize;
