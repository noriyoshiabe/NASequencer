#pragma once

#include "ParseContext.h"

typedef struct _Driver {
    ParseResult *(*parse)(void *self, const char *filepath);
    void (*destroy)(void *self);
} Driver;

typedef Driver *(*DriverFactory)(ParseContext *context);
