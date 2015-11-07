#include "NAMidiParser.h"
#include "NAUtil.h"
#include "NAArray.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
    ParseResult result = {};
    result.filepaths = NAArrayCreate(4, NADescriptionCString);
    
    Parser *parser = NAMidiParserCreate(&result);

    char *fullpath = NAUtilGetRealPath(argv[1]);

    bool success = parser->parseFile(parser, fullpath);
    if (success) {
        SequenceDump(result.sequence, 0);
    }
    else {
        fprintf(stderr, "parse error. %d:%s %s - %d:%d\n",
                result.error.kind,
                ParseErrorKind2String(result.error.kind),
                result.error.location.filepath,
                result.error.location.line,
                result.error.location.column);
    }

    if (result.sequence) {
        SequenceRelease(result.sequence);
    }

    NAArrayTraverse(result.filepaths, free);
    NAArrayDestroy(result.filepaths);

    parser->destroy(parser);
    free(fullpath);
 
    return success ? 0 : 1;
}
