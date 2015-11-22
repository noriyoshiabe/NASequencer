#include "Parser.h"
#include "NAMidiParser.h"
#include "ABCParser.h"
#include "NAUtil.h"

#include <stdlib.h>
#include <string.h>

struct _Parser {
    DSLParser *parser;
    SequenceBuilder *builder;
    ParseInfo *info;
    ParserCallbacks *callbacks;
    void *receiver;
};

static ParserCallbacks ParserDSLParserCallbacks;

static DSLParserFactory FindDSLParserFactory(const char *filepath)
{
    const struct {
        const char *extenstion;
        DSLParserFactory factory;
    } parserTable[] = {
        {"namidi", NAMidiParserCreate},
        {"abc", ABCParserCreate},
    };

    for (int i = 0; i < sizeof(parserTable) / sizeof(parserTable[0]); ++i) {
        if (0 == strcmp(parserTable[i].extenstion, NAUtilGetFileExtenssion(filepath))) {
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

    DSLParserFactory factory = FindDSLParserFactory(filepath);
    if (!factory) {
        ParseError *error = ParseErrorCreate(&((ParseLocation){filepath, 0, 0}), ParseErrorKindGeneral, GeneralParseErrorUnsupportedFileType);
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
    ParseError *_error = ParseErrorCreate(&error->location, error->kind, error->error);
    NAArrayAppend(self->info->errors, _error);
    self->callbacks->onParseError(self->receiver, _error);
}

static ParserCallbacks ParserDSLParserCallbacks = {
    ParserDSLParserOnReadFile,
    ParserDSLParserOnParseError,
};


const char *ParseError2String(const ParseError *error)
{
    switch (error->kind) {
    case ParseErrorKindGeneral:
        return GeneralParseError2String(error->error);
    case ParseErrorKindNAMidi:
        return NAMidiParseError2String(error->error);
    case ParseErrorKindABC:
        return ABCParseError2String(error->error);
    case ParseErrorKindMML:
        return "TODO";
    default:
        return "Unknown error";
    }
}

ParseError *ParseErrorCreate(const ParseLocation *location, ParseErrorKind kind, int error)
{
    ParseError *self = calloc(1, sizeof(ParseError));
    if (location->filepath) {
        self->location.filepath = strdup(location->filepath);
    }
    self->location.line = location->line;
    self->location.column = location->column;
    self->kind = kind;
    self->error = error;
    return self;
}

void ParseErrorDestroy(ParseError *self)
{
    if (self->location.filepath) {
        free((char *)self->location.filepath);
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
    }
}
