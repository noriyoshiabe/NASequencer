#include "Parser.h"
#include "ParseErrorCode.h"
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

    NAIterator *iterator = NAArrayGetIterator(info->errors);
    while (iterator->hasNext(iterator)) {
        ParseError *error = iterator->next(iterator);
        printf("[ERROR:%d] %s at %s:%d:%d\n",
                error->code,
                ParseErrorCode2String(error->code),
                error->location.filepath,
                error->location.line,
                error->location.column);

        NAIterator *iterator2 = NAArrayGetIterator(error->infos);
        while (iterator2->hasNext(iterator2)) {
            printf("    info: %s\n", iterator2->next(iterator2));
        }
    }

    SequenceDump(sequence, 0);
    SequenceRelease(sequence);
    ParseInfoRelease(info);

    return success ? 0 : 1;
}
