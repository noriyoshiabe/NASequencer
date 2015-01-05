#pragma once

#include "natype.h"

typedef struct __NATypeVtbl {
    void (*destroy)(const void *self);
    uint32_t (*hash)(const void *self);
    Boolean (*equal)(const void *self, const void *to);
    int (*compare)(const void *self, const void *to);
} NATypeVtbl;

typedef struct __NATypeCtx {
    int32_t refCount;
    uint32_t hash;
} NATypeCtx;

struct __NAType {
    NATypeVtbl *v;
    NATypeCtx ctx;
};

void *__NATypeInit(void *self);
void __NATypeDestroy(const void *self);
uint32_t __NATypeHash(const void *self);
Boolean __NATypeEqual(const void *self, const void *to);
int __NATypeCompare(const void *self, const void *to);
