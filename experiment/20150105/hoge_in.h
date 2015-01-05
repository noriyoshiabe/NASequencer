#pragma once

#include "natype_in.h"
#include "hoge.h"

typedef struct __HogeVtbl {
    NATypeVtbl _;
    void (*doHoge)(void *self);
} HogeVtbl;

struct __Hoge {
    HogeVtbl *v;
    NATypeCtx ctx;
    int hogeVal;
};
