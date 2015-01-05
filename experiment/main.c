#include <CoreFoundation/CoreFoundation.h>

// natype.h
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

typedef struct __NAType {
    NATypeVtbl *v;
    NATypeCtx ctx;
} NAType;

extern const void *NARetain(const void *self);
extern void NARelease(const void *self);
extern uint32_t NAHash(const void *self);
extern Boolean NAEqual(const void *self, const void *to);



// natype.c

void *__NATypeInit(void *self)
{
    NAType *_self = (NAType *)self;
    _self->ctx.refCount = 1;
    _self->v->hash(self);
    return _self;
}

void __NATypeDestroy(const void *self)
{
    printf("destroy 0x%08X\n", (uint32_t)self);
}

uint32_t __NATypeHash(const void *self)
{
    return (uint32_t)self >> 2;
}

const void *__NATypeRetain(const void *self)
{
    NAType *_self = (NAType *)self;
    ++_self->ctx.refCount;
    return self;
}

void __NATypeRelease(const void *self)
{
    NAType *_self = (NAType *)self;
    if (0 == --_self->ctx.refCount) {
        _self->v->destroy(self);
    }
}

Boolean __NATypeEqual(const void *self, const void *to)
{
    return self == to;
}

int __NATypeCompare(const void *self, const void *to)
{
    return self - to;
}

const void *NARetain(const void *self)
{
    return __NATypeRetain(self);
}

void NARelease(const void *self)
{
    __NATypeRelease(self);
}

uint32_t NAHash(const void *self)
{
    NAType *_self = (NAType *)self;
    return _self->v->hash(self);
}

Boolean NAEqual(const void *self, const void *to)
{
    NAType *_self = (NAType *)self;
    return _self->v->equal(self, to);
}

const void *__CFArrayRetainCallBack(CFAllocatorRef allocator, const void *value)
{
    return __NATypeRetain(value);
}

void __CFArrayReleaseCallBack(CFAllocatorRef allocator, const void *value)
{
    __NATypeRelease(value);
}

CFStringRef __CFArrayCopyDescriptionCallBack(const void *value)
{
    return NULL;
}

Boolean __CFArrayEqualCallBack(const void *value1, const void *value2)
{
    return NAEqual(value1, value2);
}

const CFArrayCallBacks __CFArrayCallBacks = {
    0,
    __CFArrayRetainCallBack,
    __CFArrayReleaseCallBack,
    __CFArrayCopyDescriptionCallBack,
    __CFArrayEqualCallBack,
};

typedef struct __HogeVtbl {
    NATypeVtbl _;
    void (*doHoge)();
} HogeVtbl;

typedef struct __Hoge {
    HogeVtbl *v;
    NATypeCtx ctx;
    int hogeVal;
} Hoge;

typedef struct __Hage {
    HogeVtbl *v;
    NATypeCtx ctx;
    int hageVal;
} Hage;

static void __doHoge()
{
    printf("-- hoge!!\n");
}

static void __doHage()
{
    printf("-- HAGE!!\n");
}

HogeVtbl __hogeVtbl  = {
    {
        __NATypeDestroy,
        __NATypeHash,
        __NATypeEqual,
        __NATypeCompare,
    },
    __doHage,
};

HogeVtbl __hageVtbl  = {
    {
        __NATypeDestroy,
        __NATypeHash,
        __NATypeEqual,
        __NATypeCompare,
    },
    __doHage,   
};

Hoge *HogeCrate()
{
    Hoge *self = malloc(sizeof(Hoge));
    self->v = &__hogeVtbl;
    return __NATypeInit(self);
}

Hoge *HageCrate()
{
    Hoge *self = malloc(sizeof(Hoge));
    self->v = &__hageVtbl;
    return __NATypeInit(self);
}
CFArrayRef CFArrayCreate(CFAllocatorRef allocator, const void **values, CFIndex numValues, const CFArrayCallBacks *callBacks);

int main(int argc, char **argv)
{
    Hoge *hoge = HogeCrate();
    Hoge *hage = HageCrate();

    CFMutableArrayRef array = CFArrayCreateMutable(NULL, 0, &__CFArrayCallBacks);
    CFArrayAppendValue(array, hoge);
    CFArrayAppendValue(array, hage);

    NARelease(hoge);
    NARelease(hage);

    printf("--- NAReleased\n");

    CFRelease(array);

    printf("--- CAReleased\n");

    return 0;
}
