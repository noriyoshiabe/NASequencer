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
    uint32_t (*hash)(const void *self) = NAVtblLookup(self, NAType)->hash;
    return hash ? hash(self) : (uint32_t)self >> 2;
}

bool NAEqual(const void *self, const void *to)
{
    bool (*equal)(const void *, const void *) = NAVtblLookup(self, NAType)->equalTo;
    return equal ? equal(self, to) : NAHash(self) == NAHash(to);
}

int NACompare(const void *self, const void *to)
{
    int (*compare)(const void *, const void *) = NAVtblLookup(self, NAType)->compare;
    return compare ? compare(self, to) : (int)self - (int)to;
}
