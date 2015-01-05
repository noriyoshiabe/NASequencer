#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct __NAVtbl {
    int *typeID;
    void *vtbl;
} NAVtbl;

typedef struct __NAClass {
    int *typeID;
    size_t size;
    NAVtbl *pvtbl;
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

extern void *NATypeInitDefault(void *self, ...);
extern void NATypeDestroyDefault(void *self);
extern uint32_t NATypeHashDefault(const void *self);
extern bool NATypeEqualDefault(const void *self, const void *to);
extern int NATypeCompareDefault(const void *self, const void *to);

extern NAClass NATypeClass;
extern int NATypeID;

static inline void *NATypeVtblLookup(NAVtbl *pvtbl, int *typeID)
{
    do {
        if (pvtbl->typeID == typeID) {
            return pvtbl->vtbl;
        }
    } while ((++pvtbl)->typeID);

    return 0;
}

#define __NATypeVtblList(self) (((NAType *)self)->clazz->pvtbl)
#define NATypeLookup(self, type) ((type##Vtbl *)NATypeVtblLookup(__NATypeVtblList(self), &type##ID))
#define NATypeNew(type, ...) ((NATypeVtbl *)NATypeVtblLookup(type##Class.pvtbl, &NATypeID))->init(NATypeAlloc(&type##Class), __VA_ARGS__)

#define NACast(self, type) ((type *)self)
#define NAGetType(self) (NACast(self, NAType))
#define NAGetClass(self) ((NAGetType(self)->clazz))

static inline void *NATypeAlloc(NAClass *clazz)
{
    NAType *self = calloc(1, clazz->size);
    self->clazz = clazz;
    self->refCount = 1;
    return self;
}

static inline uint32_t NAHash(const void *self)
{
    return NATypeLookup(self, NAType)->hash(self);
}

static inline bool NAEqualTo(const void *self, const void *to)
{
    return NATypeLookup(self, NAType)->equalTo(self, to);
}

static inline int NACompare(const void *self, const void *to)
{
    return NATypeLookup(self, NAType)->compare(self, to);
}

static inline const void *NARetain(const void *self)
{
    return ++NAGetType(self)->refCount, self;
}

static inline void NARelease(const void *self)
{
    0 == --NAGetType(self)->refCount ? NATypeLookup(self, NAType)->destroy((void *)self), free((void *)self) : (void *)0;
}

static inline int16_t NARefCount(const void *self)
{
    return NAGetType(self)->refCount;
}