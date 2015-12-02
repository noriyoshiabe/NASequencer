#pragma once

#include "Node.h"
#include "ParseContext.h"

typedef struct _Analyzer {
    Node *(*process)(Node *node);
    void (*destroy)(void *self);
} Analyzer;

typedef Analyzer *(*AnalyzerFactory)(ParseContext *context);
