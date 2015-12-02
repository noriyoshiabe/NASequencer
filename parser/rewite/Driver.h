#pragma once

#include "ParseResult.h"
#include "SequenceBuilder.h"

typedef struct _Driver {
    ParseResult *(*parse)(SequenceBuilder *builder);
    void (*destroy)(void *self);
} Driver;
