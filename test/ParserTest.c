#include "Parser.h"
#include "Sequence.h"
#include "SequenceBuilderImpl.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    ParseInfo *info = NULL;

    SequenceBuilder *builder = SequenceBuilderCreate();
    Parser *parser = ParserCreate(builder);
    Sequence *sequence = ParserParseFile(parser, argv[1], &info);
    ParserDestroy(parser);

    bool success = NAArrayIsEmpty(info->errors);

    SequenceDump(sequence, 0);
    SequenceRelease(sequence);
    ParseInfoRelease(info);

    return success ? 0 : 1;
}
