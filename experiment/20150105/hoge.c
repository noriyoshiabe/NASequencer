#include "hoge_in.h"

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
