#pragma once

#include "ParseContext.h"

typedef struct _Driver {
    void *(*parse)(void *self, const char *filepath, ParseInfo **info);
    void (*destroy)(void *self);
} Driver;

typedef Driver *(*DriverFactory)(ParseContext *context);
