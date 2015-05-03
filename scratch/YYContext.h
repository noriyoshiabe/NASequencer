#pragma once

#include <Expression.h>
#include <stdbool.h>

typedef struct _YYContext {
    bool eofReached;
    ParseLocation location;
} YYContext;
