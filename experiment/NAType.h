#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct __NAVtblEntry {
    char *typeID;
    void *vtbl;
} NAVtblEntry;

typedef struct __NAClass {
    char *typeID;
    size_t size;
    NAVtblEntry *pvEntry;
} NAClass;

typedef struct __NAType {
    NAClass *clazz;
    int32_t refCount;
} NAType;

typedef struct __NATypeVtbl {
    void *(*init)(void *self, ...);
    void (*destroy)(void *self);
    uint32_t (*hash)(const void *self);
    bool (*equalTo)(const void *self, const void *to);
    int (*compare)(const void *self, const void *to);
} NATypeVtbl;

extern char NATypeID[];

extern void *NATypeInitDefault(void *self, ...);
extern void NATypeDestroyDefault(void *self);
extern uint32_t NATypeHashDefault(const void *self);
extern bool NATypeEqualDefault(const void *self, const void *to);
extern int NATypeCompareDefault(const void *self, const void *to);

static inline void *__NAVtblLookup(NAVtblEntry *pvEntry, char *typeID)
{
    do {
        if (pvEntry->typeID == typeID) {
            return pvEntry->vtbl;
        }
    } while ((++pvEntry)->typeID);

    fprintf(stderr, "Virtual function table of [%s] is not found.\n", typeID);
    abort();
}

#define NAVtblLookup(self, type) ((type##Vtbl *)__NAVtblLookup(((NAType *)self)->clazz->pvEntry, type##ID))
#define NATypeNew(type, ...) ((NATypeVtbl *)__NAVtblLookup(type##Class.pvEntry, NATypeID))->init(NATypeAlloc(&type##Class), __VA_ARGS__)

static inline void *NATypeAlloc(NAClass *clazz)
{
    NAType *self = calloc(1, clazz->size);
    self->clazz = clazz;
    self->refCount = 1;
    return self;
}

static inline uint32_t NAHash(const void *self)
{
    return NAVtblLookup(self, NAType)->hash(self);
}

static inline bool NAEqualTo(const void *self, const void *to)
{
    return NAVtblLookup(self, NAType)->equalTo(self, to);
}

static inline int NACompare(const void *self, const void *to)
{
    return NAVtblLookup(self, NAType)->compare(self, to);
}

static inline const void *NARetain(const void *self)
{
    return ++((NAType *)self)->refCount, self;
}

static inline void NARelease(const void *self)
{
    0 == --((NAType *)self)->refCount ? NAVtblLookup(self, NAType)->destroy((void *)self), free((void *)self) : (void *)0;
}

static inline int16_t NARefCount(const void *self)
{
    return ((NAType *)self)->refCount;
}
