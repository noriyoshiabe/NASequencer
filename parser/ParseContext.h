#pragma once

#include "ParseInfo.h"
#include "SequenceBuilder.h"
#include "NASet.h"

typedef struct _ParseContext ParseContext;
struct _ParseContext {
    SequenceBuilder *builder;
    const char *includePath;

    NAArray *filepaths;
    NAArray *errors;

    void (*appendError)(ParseContext *self, const FileLocation *location, int code, ...);
    char *(*appendFile)(ParseContext *self, const char *filepath);
    void *(*buildResult)(ParseContext *self, ParseInfo **info);

    NASet *_fileSet;
    NASet *_errorSignatureSet;
};

extern ParseContext *ParseContextCreate(SequenceBuilder *builder, const char *includePath);
extern void ParseContextDestroy(ParseContext *self);
