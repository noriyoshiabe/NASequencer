#pragma once

#include "Node.h"
#include "ParseContext.h"

typedef struct _Analyzer {
    Node *(*process)(void *self, Node *node);
    void (*destroy)(void *self);

    void *self;
} Analyzer;

typedef Analyzer *(*AnalyzerFactory)(ParseContext *context);
