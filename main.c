/*
 * main.c file
 */
 
#include "DSLParser.h"
#include "stdio.h"
 
int main(int argc, char **argv)
{
    DSLParser *parser = DSLParserCreate();
    if (!DSLParserParseFile(parser, argv[1])) {
        printf("ERROR=%d\n", parser->error);
    }
    else {
        DSLParserDumpExpression(parser);
        DSLParserDestroy(parser);
    }
 
    return 0;
}
