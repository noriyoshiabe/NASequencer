#include "Parser.h"
#include "NAMidiParser.h"
#include "NAUtil.h"

#include <stdlib.h>

typedef enum {
    SyntaxNAMidi,
} Syntax;

static const struct {
    Syntax syntax;
    ParserFactory factory;
} ParserTable[] = {
    {SyntaxNAMidi, NAMidiParserCreate},
};


bool ParserParseFile(const char *filepath, ParseResult *result)
{
    Parser *parser = ParserTable[0].factory(result);
    // TODO filetype check
    if (!parser) {
        result->error.kind = ParseErrorKindUnsupportedFileType;
        return false;
    }

    result->filepaths = NAArrayCreate(4, NADescriptionCString);
    char *fullpath = NAUtilGetRealPath(filepath);

    bool success = parser->parseFile(parser, fullpath);

    parser->destroy(parser);
    free(fullpath);

    return success;
}
