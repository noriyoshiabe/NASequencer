#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

extern uint32_t NAHash(const void *self);
extern bool NAEqualTo(const void *self, const void *to);
extern int NACompare(const void *self, const void *to);
extern const void *NARetain(const void *self);
extern void NARelease(const void *self);
extern int16_t NARefCount(const void *self);

typedef struct __NAVtbl {
    int *typeID;
    void *vtbl;
} NAVtbl;

typedef struct __NAClass {
    int *typeID;
    const char *name;
    size_t size;
    void *(*init)(void *self, ...);
    void (*destroy)(void *self);
    NAVtbl *vtbl;
} NAClass;

typedef struct __NATypeCtx {
    NAClass *clazz;
    uint16_t hdr;
    int16_t refCount;
    uint32_t hash;
} NATypeCtx;

typedef struct __NATypeVtbl {
    uint32_t (*hash)(const void *self);
    bool (*equalTo)(const void *self, const void *to);
    int (*compare)(const void *self, const void *to);
} NATypeVtbl;

extern void *NATypeAlloc(NAClass *clazz);
extern void *NATypeResetHash(void *self);
#define NATypeNew(type, ...) (NATypeResetHash(type##Class.init(NATypeAlloc(&type##Class), __VA_ARGS__)))

extern void *NATypeVtblLookup(const void *self, NAClass *clazz);
#define NATypeLookup(self, type) ((type##Vtbl *)NATypeVtblLookup(self, &type##Class))

extern void *__NATypeInit(void *self, ...);
extern void __NATypeDestroy(void *self);
extern uint32_t __NATypeHash(const void *self);
extern bool __NATypeEqualTo(const void *self, const void *to);
extern int __NATypeCompare(const void *self, const void *to);

extern NAClass NATypeClass;
extern int NATypeID;

#define NACast(self, type) ((type *)self)
#define NAGetCtx(self) (NACast(self, NATypeCtx))
#define NAGetClass(self) ((NAGetCtx(self)->clazz))

#define isNAType(self) (NAGetCtx(self)->hdr == 0x4E41)

#include <stdio.h>
#define __Trace printf("---- %s %d\n", __FILE__, __LINE__)
