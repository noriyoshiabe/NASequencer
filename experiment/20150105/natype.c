#include "natype_in.h"

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
