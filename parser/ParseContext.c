#include "ParseContext.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static void ParseContextAppendError(ParseContext *self, const FileLocation *location, int code, ...)
{
    ParseError *error = ParseErrorCreate();

    error->code = code;

    if (location) {
        error->location.filepath = strdup(location->filepath);
        error->location.line = location->line;
        error->location.column = location->column;
    }

    va_list argList;
    va_start(argList, code);

    const char *str;
    for (int i = 0; i < 4 && (str = va_arg(argList, const char *)); ++i) {
        NAArrayAppend(error->infos, strdup(str));
    }

    va_end(argList);

    NAArrayAppend(self->errors, error);
}

static void ParseContextAppendFile(ParseContext *self, const char *filepath)
{
    if (!NASetContains(self->_fileSet, (char *)filepath)) {
        char *_filepath = strdup(filepath);
        NASetAdd(self->_fileSet, _filepath);
        NAArrayAppend(self->filepaths, _filepath);
    }
}

static void *ParseContextBuildResult(ParseContext *self, ParseInfo **info)
{
    if (info) {
        *info = ParseInfoCreate();

        (*info)->filepaths = self->filepaths;
        self->filepaths = NULL;

        (*info)->errors = self->errors;
        self->errors = NULL;
    }

    return self->builder->build(self->builder);
}

ParseContext *ParseContextCreate(SequenceBuilder *builder)
{
    ParseContext *self = calloc(1, sizeof(ParseContext));
    self->builder = builder;
    self->filepaths = NAArrayCreate(4, NADescriptionCString);
    self->errors = NAArrayCreate(4, NADescriptionAddress);
    self->_fileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->appendError = ParseContextAppendError;
    self->appendFile = ParseContextAppendFile;
    self->buildResult = ParseContextBuildResult;
    return self;
}

void ParseContextDestroy(ParseContext *self)
{
    if (self->filepaths) {
        NAArrayTraverse(self->filepaths, free);
        NAArrayDestroy(self->filepaths);
    }

    if (self->errors) {
        NAArrayTraverse(self->errors, ParseErrorDestroy);
        NAArrayDestroy(self->errors);
    }

    NASetDestroy(self->_fileSet);
    free(self);
}
