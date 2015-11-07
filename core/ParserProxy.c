#include "ParserProxy.h"
#include "NAMidiParser.h"
#include "NAMidiSequenceBuilder.h"
#include "NAUtil.h"

#include <stdlib.h>
#include <string.h>

struct _ParserProxy {
    ParseError *error;
    NAArray *filepaths;
};

static ParserCallbacks ParserProxyParserCallbacks;

static ParserFactory FindParserFactory(const char *filepath)
{
    const struct {
        const char *extenstion;
        ParserFactory factory;
    } parserTable[] = {
        {"namidi", NAMidiParserCreate},
    };

    for (int i = 0; i < sizeof(parserTable) / sizeof(parserTable[0]); ++i) {
        if (0 == strcmp(parserTable[i].extenstion, NAUtilGetFileExtenssion(filepath))) {
            return parserTable[i].factory;
        }
    }

    return NULL;
}

extern ParserProxy *ParserProxyCreate()
{
    return calloc(0, sizeof(ParserProxy));
}

void ParserProxyDestroy(ParserProxy *self)
{
    free(self);
}

bool ParserProxyParseFile(ParserProxy *self, const char *filepath, Sequence **sequence, ParseError *error, NAArray *filepaths)
{
    ParserFactory factory = FindParserFactory(filepath);
    if (!factory) {
        error->kind = ParseErrorKindGeneral;
        error->error = GeneralParseErrorUnsupportedFileType;
        error->location.filepath = filepath;
        return false;
    }

    self->error = error;
    self->filepaths = filepaths;

    SequenceBuilder *builder = NAMidiSequenceBuilderCreate();
    Parser *parser = factory(builder, &ParserProxyParserCallbacks, self);
    char *fullpath = NAUtilGetRealPath(filepath);

    bool success = parser->parseFile(parser, fullpath);

    if (success) {
        *sequence = builder->build(builder);
    }

    builder->destroy(builder);
    parser->destroy(parser);
    free(fullpath);

    return success;
}

static void ParserProxyParserOnReadFile(void *receiver, const char *filepath)
{
    ParserProxy *self = receiver;
    if (self->filepaths) {
        NAArrayAppend(self->filepaths, strdup(filepath));
    }
}

static void ParserProxyParserOnParseError(void *receiver, ParseError *error)
{
    ParserProxy *self = receiver;
    if (self->error) {
        memcpy(self->error, error, sizeof(ParseError));
    }
}

static ParserCallbacks ParserProxyParserCallbacks = {
    ParserProxyParserOnReadFile,
    ParserProxyParserOnParseError,
};
