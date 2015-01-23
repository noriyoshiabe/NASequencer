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
        printf("DSLParserError=%d %s\n", error.kind, error.filepath);
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
        DSLParserDumpExpression(expression);

        ASTParserError error;
        Sequence *sequence;

        if (!(sequence = ASTParserParseExpression(expression, argv[1], &error))) {
            printf("ASTParserError=%d %s\n", error.kind, error.filepath);
            printf("\t%d %d %d %d %s\n",
                    error.expression->location.firstLine,
                    error.expression->location.firstColumn,
                    error.expression->location.lastLine,
                    error.expression->location.lastColumn,
                    error.message);
        }
        else {
            CFStringRef cfString = NADescription(sequence);
            CFShow(cfString);
            CFRelease(cfString);
            NARelease(sequence);
        }

        DSLParserDeleteExpression(expression);
    }

    return 0;
}
