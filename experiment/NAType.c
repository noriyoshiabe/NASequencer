#include <NAType.h>
#include <NAString.h>

char NATypeID[] = "NAType";

void *__NATypeAlloc(NAClass *clazz)
{
    NAType *self = calloc(1, clazz->size);
    self->clazz = clazz;
    self->refCount = 1;
    return self;
}

static void *__NANOPInit(void *self, ...)
{
    return self;
}

void *(*__NAFindInit(NAClass *clazz))(void *self, ...)
{
    void *(*init)(void *, ...) = __NAVtblLookup(clazz, NATypeID);
    return init ? init : __NANOPInit;
}

void *__NAVtblLookup(NAClass *clazz, char *typeID)
{
     NAVtblEntry *pv = clazz->pvEntry;
    do {
        if (pv->typeID == typeID) {
            return pv->vtbl;
        }
    } while ((++pv)->typeID);

    fprintf(stderr, "Virtual function table of [%s] is not found.\n", typeID);
    abort();
}

const void *NARetain(const void *self)
{
    return ++((NAType *)self)->refCount, self;
}

void NARelease(const void *self)
{
    0 == --((NAType *)self)->refCount ? NAVtbl(self, NAType)->destroy((void *)self), free((void *)self) : (void *)0;
}

int16_t NARefCount(const void *self)
{
    return ((NAType *)self)->refCount;
}

uint32_t NAHash(const void *self)
{
    uint32_t (*hash)(const void *self) = NAVtbl(self, NAType)->hash;
    return hash ? hash(self) : (uint32_t)self >> 2;
}

bool NAEqual(const void *self, const void *to)
{
    bool (*equal)(const void *, const void *) = NAVtbl(self, NAType)->equalTo;
    return equal ? equal(self, to) : NAHash(self) == NAHash(to);
}

int NACompare(const void *self, const void *to)
{
    int (*compare)(const void *, const void *) = NAVtbl(self, NAType)->compare;
    return compare ? compare(self, to) : (int)self - (int)to;
}
