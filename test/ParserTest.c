#include "Parser.h"
#include "Sequence.h"
#include "SequenceBuilderImpl.h"

#include <stdio.h>
#include <stdlib.h>

static ParserCallbacks ParserTestCallbacks;

int main(int argc, char **argv)
{
    Sequence *sequence = NULL;
    ParseInfo *info = NULL;

    SequenceBuilder *builder = SequenceBuilderCreate();
    Parser *parser = ParserCreate(builder, &ParserTestCallbacks, NULL);
    bool success = ParserParseFile(parser, argv[1], (void **)&sequence, &info);
    ParserDestroy(parser);

    SequenceDump(sequence, 0);
    SequenceRelease(sequence);
    ParseInfoRelease(info);

    return success ? 0 : 1;
}

static void ParserTestOnReadFile(void *receiver, const char *filepath)
{
    fprintf(stderr, "reading %s\n", filepath);
}

static void ParserTestOnParseError(void *receiver, const ParseError *error)
{
    fprintf(stderr, "parse error. %s - %s:%d:%d\n", ParseError2String(error), error->location.filepath, error->location.line, error->location.column);
}

static ParserCallbacks ParserTestCallbacks = {
    ParserTestOnReadFile,
    ParserTestOnParseError,
};

