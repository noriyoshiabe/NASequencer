#include "ParserProxy.h"
#include "NAUtil.h"

#include <stdlib.h>
#include <string.h>

struct _ParserProxy {
    ParseError *error;
    NAArray *filepaths;
};

static ParserFactory FindParserFactory(const char *filepath)
{
    const struct {
        const char *extenstion;
        ParserFactory factory;
    } parserTable[] = {
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

    SequenceBuilder *builder = SequenceBuilderCreate();
    // TODO callback;
    Parser *parser = factory(builder, NULL, self);
    char *fullpath = NAUtilGetRealPath(filepath);

    bool success = parser->parseFile(parser, fullpath);

    if (success) {
        *sequence = SequenceBuilderBuild(builder);
    }

    SequenceBuilderDestroy(builder);
    parser->destroy(parser);
    free(fullpath);

    return success;
}
