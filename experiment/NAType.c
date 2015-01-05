#include <NAType.h>
#include <NAString.h>

#include <stdlib.h>

uint32_t NAHash(const void *self)
{
    return NAGetCtx(self)->hash = 0 != NAGetCtx(self)->hash ? NAGetCtx(self)->hash : NATypeLookup(self, NAType)->hash(self);
}

bool NAEqualTo(const void *self, const void *to)
{
    return NATypeLookup(self, NAType)->equalTo(self, to);
}

int NACompare(const void *self, const void *to)
{
    return NATypeLookup(self, NAType)->compare(self, to);
}

const void *NARetain(const void *self)
{
    return ++NAGetCtx(self)->refCount, self;
}

void NARelease(const void *self)
{
    0 == --NAGetCtx(self)->refCount ? NATypeLookup(self, NAType)->destroy((void *)self), free((void *)self) : (void *)0;
}

int16_t NARefCount(const void *self)
{
    return NAGetCtx(self)->refCount;
}

void *NATypeAlloc(NAClass *clazz)
{
    NAType *self = calloc(1, clazz->size);
    self->clazz = clazz;
    self->hdr = 0x4E41;
    self->refCount = 1;
    return self;
}

void *NATypeResetHash(void *self)
{
    NAGetCtx(self)->hash = 0;
    return NAHash(self), self;
}

void *NATypeVtblLookup(NAVtbl *pvtbl, NAClass *clazz)
{
    do {
        if (pvtbl->typeID == clazz->typeID) {
            return pvtbl->vtbl;
        }
    } while ((++pvtbl)->typeID);

    return 0;
}

void *__NATypeInit(void *self, ...)
{
    return self;
}

void __NATypeDestroy(void *self)
{
}

uint32_t __NATypeHash(const void *self)
{
    return (uint32_t)self >> 2;
}

bool __NATypeEqualTo(const void *self, const void *to)
{
    return NAHash(self) == NAHash(to);
}

int __NATypeCompare(const void *self, const void *to)
{
    return (int)self - (int)to;
}

static NATypeVtbl typeVtbl = {
    __NATypeInit,
    __NATypeDestroy,
    __NATypeHash,
    __NATypeEqualTo,
    __NATypeCompare,
};

static NAVtbl vtbls[] = {
    {&NATypeID, &typeVtbl},
    {NULL, 0},
};

NAClass NATypeClass = {
    &NATypeID,
    sizeof(NAType),
    vtbls,
};

int NATypeID;
