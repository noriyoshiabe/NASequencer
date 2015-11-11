#include "ParserProxy.h"
#include "NAMidiParser.h"
#include "SequenceBuilder.h"
#include "NAUtil.h"
#include "NAArray.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    ParserProxy *parser = ParserProxyCreate();
    Sequence *sequence = NULL;
    ParseError error = {};
    NAArray *filepaths = NAArrayCreate(4, NADescriptionCString);
    char *filepath = NAUtilGetRealPath(argv[1]);

    bool success = ParserProxyParseFile(parser, filepath, &sequence, &error, filepaths);
    if (!success) {
        const char *detail;

        switch (error.kind) {
        case ParseErrorKindGeneral:
            detail = GeneralParseError2String(error.error);
            break;
        case ParseErrorKindNAMidi:
            detail = NAMidiParseError2String(error.error);
            break;
        case ParseErrorKindABC:
        case ParseErrorKindMML:
            detail = "TODO";
            break;
        default:
            break;
        }

        fprintf(stderr, "parse error. %s:%s - %s:%d:%d\n",
                ParseErrorKind2String(error.kind), detail,
                error.location.filepath, error.location.line, error.location.column);
    }
    else {
        SequenceDump(sequence, 0);
        SequenceRelease(sequence);

        printf("\nfrom:\n");

        NAIterator *iterator = NAArrayGetIterator(filepaths);
        while (iterator->hasNext(iterator)) {
            printf("  %s\n", iterator->next(iterator));
        }
    }


    NAArrayTraverse(filepaths, free);
    NAArrayDestroy(filepaths);
    ParserProxyDestroy(parser);

    return success ? 0 : 1;
}
