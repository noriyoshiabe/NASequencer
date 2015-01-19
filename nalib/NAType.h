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

#define NATypeNew(type, ...) ((NATypeVtbl *)__NAFindInit(&type##Class))->init(__NATypeAlloc(&type##Class), __VA_ARGS__)
#define NAVtbl(self, type) ((type##Vtbl *)__NAVtblLookup(((NAType *)self)->clazz, type##ID))

extern const void *NARetain(const void *self);
extern void NARelease(const void *self);
extern int16_t NARefCount(const void *self);

extern uint32_t NAHash(const void *self);
extern bool NAEqual(const void *self, const void *to);
extern int NACompare(const void *self, const void *to);

extern void *__NATypeAlloc(NAClass *clazz);
extern void *(*__NAFindInit(NAClass *clazz))(void *self, ...);
extern void *__NAVtblLookup(NAClass *clazz, char *typeID);
