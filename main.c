/*
 * main.c file
 */
 
#include "DSLParser.h"
#include "ASTParser.h"
#include "Sequence.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char **argv)
{
    DSLParserError error;
    Expression *expression;

    if (!(expression = DSLParserParseFile(argv[1], &error))) {
        printf("ERROR=%d %s\n", error.kind, error.filepath);
        if (DSLPARSER_PARSE_ERROR == error.kind) {
            printf("\t%d %d %d %d %s\n",
                    error.location.firstLine,
                    error.location.firstColumn,
                    error.location.lastLine,
                    error.location.lastColumn,
                    error.message);
        }
    }
    else {
        ASTParserError error;
        Sequence *sequence = ASTParserParseExpression(expression, argv[1], &error);
        if (sequence) {
            CFStringRef cfString = NADescription(sequence);
            CFShow(cfString);
            CFRelease(cfString);
            NARelease(sequence);
        }

        DSLParserDumpExpression(expression);
        DSLParserDeleteExpression(expression);
    }

    return 0;
}
