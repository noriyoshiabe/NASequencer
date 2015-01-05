#include <CoreFoundation/CoreFoundation.h>

// natype.h
extern const void *NARetain(const void *self);
extern void NARelease(const void *self);
extern uint32_t NAHash(const void *self);
extern Boolean NAEqual(const void *self, const void *to);

// natype_in.h
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

void *__NATypeInit(void *self);
void __NATypeDestroy(const void *self);
uint32_t __NATypeHash(const void *self);
Boolean __NATypeEqual(const void *self, const void *to);
int __NATypeCompare(const void *self, const void *to);

// natype.c
// #include natype_in.h

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

static const void *__NATypeRetain(const void *self)
{
    NAType *_self = (NAType *)self;
    ++_self->ctx.refCount;
    return self;
}

static void __NATypeRelease(const void *self)
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

// hoge.h

extern void *HogeCrate();
extern void HogeDoHoge(void *self);

// hoge_in.h
typedef struct __HogeVtbl {
    NATypeVtbl _;
    void (*doHoge)(void *self);
} HogeVtbl;

typedef struct __Hoge {
    HogeVtbl *v;
    NATypeCtx ctx;
    int hogeVal;
} Hoge;

// hoge.c

void HogeDoHoge(void *self)
{
    Hoge *_self = self;
    _self->v->doHoge(self);
}

static void __doHoge(void *self)
{
    Hoge *_self = self;
    printf("-- hoge!! hogeVal=%d\n", _self->hogeVal);
}

HogeVtbl __hogeVtbl  = {
    {
        __NATypeDestroy,
        __NATypeHash,
        __NATypeEqual,
        __NATypeCompare,
    },
    __doHoge,
};

void *HogeCrate()
{
    Hoge *self = malloc(sizeof(Hoge));
    self->v = &__hogeVtbl;
    self->hogeVal = 1;
    return __NATypeInit(self);
}


// hage.h
extern void *HageCrate();

// hage.c

typedef struct __Hage {
    Hoge base;
    int hageVal;
} Hage;

static void __doHage(void *self)
{
    Hage *_self = self;
    printf("-- HAGE!! hogeVal=%d hageVal=%d\n", _self->base.hogeVal, _self->hageVal);
}

HogeVtbl __hageVtbl  = {
    {
        __NATypeDestroy,
        __NATypeHash,
        __NATypeEqual,
        __NATypeCompare,
    },
    __doHage,   
};

void *HageCrate()
{
    Hage *self = malloc(sizeof(Hoge));
    self->base.v = &__hageVtbl;
    self->base.hogeVal = 2;
    self->hageVal = 3;
    return __NATypeInit(self);
}

int main(int argc, char **argv)
{
    Hoge *hoge = HogeCrate();
    Hoge *hage = HageCrate();

    CFMutableArrayRef array = CFArrayCreateMutable(NULL, 0, &__CFArrayCallBacks);
    CFArrayAppendValue(array, hoge);
    CFArrayAppendValue(array, hage);

    HogeDoHoge(hoge);
    HogeDoHoge(hage);

    NARelease(hoge);
    NARelease(hage);

    printf("--- NAReleased\n");

    CFRelease(array);

    printf("--- CAReleased\n");

    return 0;
}
