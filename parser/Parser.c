#include "Parser.h"
#include "NAMidiParser.h"
#include "ABCParser.h"
#include "NAUtil.h"

#include <stdlib.h>
#include <string.h>

struct _Parser {
    DSLParser *parser;
    SequenceBuilder *builder;
};

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

Parser *ParserCreate(SequenceBuilder *builder)
{
    Parser *self = calloc(1, sizeof(Parser));
    self->builder = builder;
    return self;
}

void ParserDestroy(Parser *self)
{
    if (self->parser) {
        self->parser->destroy(self->parser);
    }

    free(self);
}

bool ParserParseFile(Parser *self, const char *filepath, void **sequence, void **info)
{
    bool success = false;

    DSLParserFactory factory = FindDSLParserFactory(filepath);
    if (!factory) {
        ParseError err = {{filepath, 0, 0}, ParseErrorKindGeneral, GeneralParseErrorUnsupportedFileType};
        self->builder->appendError(self->builder, &err);
    }
    else {
        self->parser = factory(self->builder);
        char *fullpath = NAUtilGetRealPath(filepath);
        success = self->parser->parseFile(self->parser, fullpath);
        free(fullpath);
    }

    *sequence = self->builder->buildSequence(self->builder);
    *info = self->builder->buildInfomation(self->builder);
    return success;
}

const char *ParseError2String(ParseError *error)
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
