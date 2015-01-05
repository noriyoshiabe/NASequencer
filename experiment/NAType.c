#include <NAType.h>
#include <NAString.h>

char NATypeID[] = "NAType";

void *NATypeAlloc(NAClass *clazz)
{
    NAType *self = calloc(1, clazz->size);
    self->clazz = clazz;
    self->refCount = 1;
    return self;
}

void *__NAVtblLookup(NAVtblEntry *pvEntry, char *typeID)
{
    do {
        if (pvEntry->typeID == typeID) {
            return pvEntry->vtbl;
        }
    } while ((++pvEntry)->typeID);

    fprintf(stderr, "Virtual function table of [%s] is not found.\n", typeID);
    abort();
}

const void *NARetain(const void *self)
{
    return ++((NAType *)self)->refCount, self;
}

void NARelease(const void *self)
{
    0 == --((NAType *)self)->refCount ? NAVtblLookup(self, NAType)->destroy((void *)self), free((void *)self) : (void *)0;
}

int16_t NARefCount(const void *self)
{
    return ((NAType *)self)->refCount;
}

uint32_t NAHash(const void *self)
{
    return NAVtblLookup(self, NAType)->hash(self);
}

bool NAEqualTo(const void *self, const void *to)
{
    return NAVtblLookup(self, NAType)->equalTo(self, to);
}

int NACompare(const void *self, const void *to)
{
    return NAVtblLookup(self, NAType)->compare(self, to);
}

void *NATypeInitEmpty(void *self, ...)
{
    return self;
}

void NATypeDestroyDefault(void *self)
{
}

uint32_t NATypeHashDefault(const void *self)
{
    return (uint32_t)self >> 2;
}

bool NATypeEqualDefault(const void *self, const void *to)
{
    return NAHash(self) == NAHash(to);
}

int NATypeCompareDefault(const void *self, const void *to)
{
    return (int)self - (int)to;
}
