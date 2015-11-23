#include "Parser.h"
#include "NAMidiParser.h"
#include "ABCParser.h"
#include "NAUtil.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

struct _Parser {
    DSLParser *parser;
    SequenceBuilder *builder;
    ParseInfo *info;
    ParserCallbacks *callbacks;
    void *receiver;
};

static ParserCallbacks ParserDSLParserCallbacks;

static DSLParserFactory FindDSLParserFactory(const char *ext)
{
    const struct {
        const char *extenstion;
        DSLParserFactory factory;
    } parserTable[] = {
        {"namidi", NAMidiParserCreate},
        {"abc", ABCParserCreate},
    };

    for (int i = 0; i < sizeof(parserTable) / sizeof(parserTable[0]); ++i) {
        if (0 == strcmp(parserTable[i].extenstion, ext)) {
            return parserTable[i].factory;
        }
    }

    return NULL;
}

Parser *ParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver)
{
    Parser *self = calloc(1, sizeof(Parser));
    self->builder = builder;
    self->callbacks = callbacks;
    self->receiver = receiver;
    return self;
}

void ParserDestroy(Parser *self)
{
    if (self->parser) {
        self->parser->destroy(self->parser);
    }

    free(self);
}

bool ParserParseFile(Parser *self, const char *filepath, void **sequence, ParseInfo **info)
{
    bool success = false;

    self->info = ParseInfoCreate();

    const char *ext = NAUtilGetFileExtenssion(filepath);
    DSLParserFactory factory = FindDSLParserFactory(ext);
    if (!factory) {
        ParseError *error = ParseErrorCreate(NULL, GeneralParseErrorUnsupportedFileType, ext, NULL);
        NAArrayAppend(self->info->errors, error);
        self->callbacks->onParseError(self->receiver, error);
    }
    else {
        self->parser = factory(self->builder, &ParserDSLParserCallbacks, self);
        char *fullpath = NAUtilGetRealPath(filepath);
        success = self->parser->parseFile(self->parser, fullpath);
        free(fullpath);
    }

    *sequence = self->builder->build(self->builder);
    *info = self->info;
    return success;
}

static void ParserDSLParserOnReadFile(void *receiver, const char *filepath)
{
    Parser *self = receiver;
    char *_filepath = strdup(filepath);
    NAArrayAppend(self->info->filepaths, _filepath);
    self->callbacks->onReadFile(self->receiver, _filepath);
}

static void ParserDSLParserOnParseError(void *receiver, const ParseError *error)
{
    Parser *self = receiver;
    NAArrayAppend(self->info->errors, (ParseError *)error);
    self->callbacks->onParseError(self->receiver, error);
}

static ParserCallbacks ParserDSLParserCallbacks = {
    ParserDSLParserOnReadFile,
    ParserDSLParserOnParseError,
};


const char *ParseError2String(const ParseError *error)
{
    switch (error->code - error->code % 1000) {
    case ParseErrorKindGeneral:
        return GeneralParseError2String(error->code);
    case ParseErrorKindNAMidi:
        return NAMidiParseError2String(error->code);
    case ParseErrorKindABC:
        return ABCParseError2String(error->code);
    case ParseErrorKindMML:
        return "TODO";
    default:
        return "Unknown error";
    }
}

ParseError *ParseErrorCreate(const ParseLocation *location, int code, ...)
{
    va_list argList;
    va_start(argList, code);
    ParseError *self = ParseErrorCreateWithArgs(location, code, argList);
    va_end(argList);

    return self;
}

ParseError *ParseErrorCreateWithArgs(const ParseLocation *location, int code, va_list argList)
{
    ParseError *self = calloc(1, sizeof(ParseError));

    if (location) {
        self->location.filepath = strdup(location->filepath);
        self->location.line = location->line;
        self->location.column = location->column;
    }

    self->code = code;

    const char *str;
    for (int i = 0; i < 4 && (str = va_arg(argList, const char *)); ++i) {
        self->infos[i] = strdup(str);
    }

    return self;
}

void ParseErrorDestroy(ParseError *self)
{
    if (self->location.filepath) {
        free(self->location.filepath);
    }

    for (int i = 0; i < 4; ++i) {
        if (!self->infos[i]) {
            break;
        }
        free(self->infos[i]);
    }

    free(self);
}

typedef struct ParseInfoImpl {
    ParseInfo info;
    int refCount;
} ParseInfoImpl;

ParseInfo *ParseInfoCreate()
{
    ParseInfoImpl *self = calloc(1, sizeof(ParseError));
    self->refCount = 1;
    self->info.filepaths = NAArrayCreate(4, NADescriptionCString);
    self->info.errors = NAArrayCreate(4, NULL);
    return (ParseInfo *)self;

}

ParseInfo *ParseInfoRetain(ParseInfo *_self)
{
    ParseInfoImpl *self = (ParseInfoImpl *)_self;
    ++self->refCount;
    return _self;
}

void ParseInfoRelease(ParseInfo *_self)
{
    ParseInfoImpl *self = (ParseInfoImpl *)_self;
    if (0 == --self->refCount) {
        NAArrayTraverse(self->info.filepaths, free);
        NAArrayTraverse(self->info.errors, ParseErrorDestroy);
        NAArrayDestroy(self->info.filepaths);
        NAArrayDestroy(self->info.errors);
        free(self);
    }
}
