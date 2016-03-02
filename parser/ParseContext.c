#include "ParseContext.h"
#include "NAStringBuffer.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static char *errorSignature(const FileLocation *location, int code, va_list argList)
{
    NAStringBuffer *buffer = NAStringBufferCreate(128);

    if (location) {
        NAStringBufferAppendFormat(buffer, "%s:%d:%d:%d", location->filepath, location->line, location->column, code);
    }
    else {
        NAStringBufferAppendFormat(buffer, ":::%d", code);
    }

    const char *str;
    while ((str = va_arg(argList, const char *))) {
        NAStringBufferAppendFormat(buffer, ":%s", str);
    }

    char *ret = NAStringBufferRetriveCString(buffer);
    NAStringBufferDestroy(buffer);
    return ret;
}

static void ParseContextAppendError(ParseContext *self, const FileLocation *location, int code, ...)
{
    va_list argList;

    va_start(argList, code);
    char *signature = errorSignature(location, code, argList);
    va_end(argList);

    if (NASetContains(self->_errorSignatureSet, signature)) {
        free(signature);
        return;
    }
    NASetAdd(self->_errorSignatureSet, signature);

    ParseError *error = ParseErrorCreate();

    error->code = code;

    if (location) {
        error->location.filepath = strdup(location->filepath);
        error->location.line = location->line;
        error->location.column = location->column;
    }

    va_start(argList, code);

    const char *str;
    while ((str = va_arg(argList, const char *))) {
        NAArrayAppend(error->infos, strdup(str));
    }

    va_end(argList);

    NAArrayAppend(self->errors, error);
}

static char *ParseContextAppendFile(ParseContext *self, const char *filepath)
{
    char *_filepath = NASetGet(self->_fileSet, (char *)filepath);
    if (!_filepath) {
        _filepath = strdup(filepath);
        NASetAdd(self->_fileSet, _filepath);
        NAArrayAppend(self->filepaths, _filepath);
    }
    return _filepath;
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

ParseContext *ParseContextCreate(SequenceBuilder *builder, const char *includePath)
{
    ParseContext *self = calloc(1, sizeof(ParseContext));
    self->builder = builder;
    self->includePath = includePath;
    self->filepaths = NAArrayCreate(4, NADescriptionCString);
    self->errors = NAArrayCreate(4, NADescriptionAddress);
    self->_fileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->_errorSignatureSet = NASetCreate(NAHashCString, NADescriptionCString);
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

    NASetTraverse(self->_errorSignatureSet, free);
    NASetDestroy(self->_errorSignatureSet);

    free(self);
}
