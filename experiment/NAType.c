#include <NAType.h>
#include <NAString.h>

#include <stdlib.h>

uint32_t NAHash(NAType *self)
{
    return NAGetCtx(self)->hash = 0 != NAGetCtx(self)->hash ? NAGetCtx(self)->hash : NATypeLookup(self, NAType)->hash(self);
}

bool NAEqualTo(NAType *self, NAType *to)
{
    return NATypeLookup(self, NAType)->equalTo(self, to);
}

int NACompare(NAType *self, NAType *to)
{
    return NATypeLookup(self, NAType)->compare(self, to);
}

NAString *NAToString(NAType *self)
{
    return NATypeLookup(self, NAType)->toString(self);
}

NAType *NAAddRef(NAType *self)
{
    return ++NAGetCtx(self)->refCount, self;
}

void NARelease(NAType *self)
{
    0 == --NAGetCtx(self)->refCount ? NAGetClass(self)->destroy(self), free(self) : (void *)0;
}

int16_t NARefCount(NAType *self)
{
    return NAGetCtx(self)->refCount;
}

NAType *NATypeAlloc(NAClass *clazz)
{
    NATypeCtx *self = calloc(1, clazz->size);
    self->clazz = clazz;
    self->hdr = 0x4E41;
    self->refCount = 1;
    return self;
}

NAType *NATypeResetHash(NAType *self)
{
    NAGetCtx(self)->hash = 0;
    return NAHash(self), self;
}

void *NATypeVtblLookup(NAType *self, NAClass *clazz)
{
    NAVtbl *pvtbl = ((NATypeCtx *)self)->clazz->vtbl;
    do {
        if (pvtbl->typeID == clazz->typeID) {
            return pvtbl->vtbl;
        }
    } while ((++pvtbl)->typeID);

    return 0;
}

NAType *__NATypeInit(NAType *self, ...)
{
    return self;
}

void __NATypeDestroy(NAType *self)
{
}

uint32_t __NATypeHash(NAType *self)
{
    return (uint32_t)self >> 2;
}

bool __NATypeEqualTo(NAType *self, NAType *to)
{
    return NAHash(self) == NAHash(to);
}

int __NATypeCompare(NAType *self, NAType *to)
{
    return (int)self - (int)to;
}

NAString *__NATypeToString(NAType *self)
{
    return NATypeNew(NAString, "<%s:%08X>", NAGetClass(self)->name, self);
}

static NATypeVtbl typeVtbl = {
    __NATypeHash,
    __NATypeEqualTo,
    __NATypeCompare,
    __NATypeToString,
};

static NAVtbl vtbl[] = {
    {&NATypeID, &typeVtbl},
    {NULL, 0},
};

NAClass NATypeClass = {
    &NATypeID,
    "NAType",
    sizeof(NATypeCtx),
    __NATypeInit,
    __NATypeDestroy,
    vtbl,
};

int NATypeID;
