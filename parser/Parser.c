#include "Parser.h"
#include "NAMidiParser.h"
#include "NAUtil.h"

#include <stdlib.h>
#include <string.h>

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


bool ParserParseFile(const char *filepath, ParseResult *result)
{
    ParserFactory factory = FindParserFactory(filepath);
    if (!factory) {
        result->error.kind = ParseErrorKindUnsupportedFileType;
        result->error.location.filepath = filepath;
        return false;
    }

    Parser *parser = factory(result);

    result->filepaths = NAArrayCreate(4, NADescriptionCString);
    char *fullpath = NAUtilGetRealPath(filepath);

    bool success = parser->parseFile(parser, fullpath);

    parser->destroy(parser);
    free(fullpath);

    return success;
}
