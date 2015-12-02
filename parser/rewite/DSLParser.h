#pragma once

#include "Node.h"
#include "ParseContext.h"

typedef struct _DSLParser {
    Node *parse(const char *filepath);
    void (*destroy)(void *self);
} DSLParser;

typedef DSLParser *(*DSLParserFactory)(ParseContext *context);
