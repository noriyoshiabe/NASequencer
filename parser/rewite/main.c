#include "Parser.h"

int main(int argc, char **argv)
{
    SequenceBuilder *builder = NULL;
    Parser *parser = ParserCreate(builder);
    ParseResult *result = ParserParseFile(parser, argv[1]);
    ParserDestroy(parser);
    return 0;
};
