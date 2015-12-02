#pragma once

#include "ParseResult.h"
#include "SequenceBuilder.h"
#include "NASet.h"

typedef struct _ParseContext ParseContext;
struct _ParseContext {
    SequenceBuilder *builder;
    NAArray *filepaths;
    NAArray *errors;

    void (*appendError)(ParseContext *self, const FileLocation *location, int code, ...);
    void (*appendFile)(ParseContext *self, const char *filepath);
    ParseResult *(*buildResult)(ParseContext *self);

    NASet *_fileSet;
};

extern ParseContext *ParseContextCreate(SequenceBuilder *builder);
extern void ParseContextDestroy(ParseContext *self);
