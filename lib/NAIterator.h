#pragma once

#include <stdbool.h>

typedef struct _NAIterator NAIterator;
struct _NAIterator {
    bool (*hasNext)(NAIterator *self);
    void *(*next)(NAIterator *self);
    void (*remove)(NAIterator *self);
};
