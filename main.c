/*
 * main.c file
 */
 
#include "DSLParser.h"
#include "Sequence.h"
#include "stdio.h"
 
int main(int argc, char **argv)
{
    DSLParser *parser = DSLParserCreate();
    if (!DSLParserParseFile(parser, argv[1])) {
        printf("ERROR=%d\n", parser->error);
    }
    else {
        DSLParserDumpExpression(parser);

        Sequence *sequence = NATypeNew(Sequence, 0);
        NARelease(sequence);

        DSLParserDestroy(parser);
    }
 
    return 0;
}
