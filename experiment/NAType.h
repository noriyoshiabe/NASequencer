#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef void NAType;
typedef struct __NAString NAString;

extern uint32_t NAHash(NAType *self);
extern bool NAEqualTo(NAType *self, NAType *to);
extern int NACompare(NAType *self, NAType *to);
extern NAString *NAToString(NAType *self);
extern NAType *NAAddRef(NAType *self);
extern void NARelease(NAType *self);
extern int16_t NARefCount(NAType *self);

typedef struct __NAVtbl {
    int *typeID;
    void *vtbl;
} NAVtbl;

typedef struct __NAClass {
    int *typeID;
    const char *name;
    size_t size;
    NAType *(*init)(NAType *self, ...);
    void (*destroy)(NAType *self);
    NAVtbl *vtbl;
} NAClass;

typedef struct __NATypeCtx {
    NAClass *clazz;
    uint16_t hdr;
    int16_t refCount;
    uint32_t hash;
} NATypeCtx;

typedef struct __NATypeVtbl {
    uint32_t (*hash)(NAType *self);
    bool (*equalTo)(NAType *self, NAType *to);
    int (*compare)(NAType *self, NAType *to);
    NAString *(*toString)(NAType *self);
} NATypeVtbl;

extern NAType *NATypeAlloc(NAClass *clazz);
extern NAType *NATypeResetHash(NAType *self);
#define NATypeNew(type, ...) (NATypeResetHash(type##Class.init(NATypeAlloc(&type##Class), __VA_ARGS__)))

extern void *NATypeVtblLookup(NAType *self, NAClass *clazz);
#define NATypeLookup(self, type) ((type##Vtbl *)NATypeVtblLookup(self, &type##Class))

extern NAType *__NATypeInit(NAType *self, ...);
extern void __NATypeDestroy(NAType *self);
extern uint32_t __NATypeHash(NAType *self);
extern bool __NATypeEqualTo(NAType *self, NAType *to);
extern int __NATypeCompare(NAType *self, NAType *to);
extern NAString *__NATypeToString(NAType *self);

extern NAClass NATypeClass;
extern int NATypeID;

#define NACast(self, type) ((type *)self)
#define NAGetCtx(self) (NACast(self, NATypeCtx))
#define NAGetClass(self) ((NAGetCtx(self)->clazz))

#define isNAType(self) (NAGetCtx(self)->hdr == 0x4E41)

#include <stdio.h>
#define __Trace printf("---- %s %d\n", __FILE__, __LINE__)
