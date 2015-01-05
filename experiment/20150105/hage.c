#include "hage.h"
#include "hoge_in.h"

struct __Hage {
    Hoge base;
    int hageVal;
};

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
