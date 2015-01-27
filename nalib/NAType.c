#include <NAType.h>
#include <string.h>

const char NATypeID[] = "NAType";

void *__NATypeAlloc(const NAClass *clazz)
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

void *(*__NAFindInit(const NAClass *clazz))(void *self, ...)
{
    void *(*init)(void *, ...) = __NAVtblLookup(clazz, NATypeID);
    return init ? init : __NANOPInit;
}

void *__NAVtblLookup(const NAClass *clazz, const char *typeID)
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

void *NARetain(void *self)
{
    return ++((NAType *)self)->refCount, self;
}

void NARelease(void *self)
{
    if (0 == --((NAType *)self)->refCount) {
        void (*destroy)(void *) = NAVtbl(self, NAType)->destroy;
        if (destroy) {
            destroy(self);
        }
        free(self);
    }
}

int16_t NARefCount(const void *self)
{
    return ((NAType *)self)->refCount;
}

uint32_t NAHash(const void *self)
{
    uint32_t (*hash)(const void *self) = NAVtbl(self, NAType)->hash;
    return hash ? hash(self) : (uint32_t)self;
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

void *NACopy(const void *_self)
{
    void *(*copy)(const void *) = NAVtbl(_self, NAType)->copy;
    if (copy) {
        return copy(_self);
    }
    else {
        const NAType *self = _self;

        void *(*init)(void *self, ...) = __NAFindInit(self->clazz);
        NAType *copied = init(__NATypeAlloc(self->clazz));
        memcpy(copied + 1, self + 1, self->clazz->size - sizeof(NAType));
        return copied;
    }
}

void *NADescription(const void *_self)
{
    void *(*description)(const void *) = NAVtbl(_self, NAType)->description;
    if (description) {
        return description(_self);
    }
    else {
        const NAType *self = _self;

        char *str = malloc(strlen(self->clazz->typeID) + sizeof(void *) + 8 + 6);
        if (8 < sizeof(void *)) {
            sprintf("<%s:0x%016X:rc=%d>", self->clazz->typeID, (uint64_t)self, self->refCount);
        }
        else {
            sprintf("<%s:0x%08X:rc=%d>", self->clazz->typeID, (uint32_t)self, self->refCount);
        }

        return str;
    }
}
