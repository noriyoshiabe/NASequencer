#include "Parser.h"

int main(int argc, char **argv)
{
    SequenceBuilder *builder = NULL;
    ParseInfo *info = NULL;
    Parser *parser = ParserCreate(builder);
    void *sequence = ParserParseFile(parser, argv[1], &info);
    ParseInfoRelease(info);
    ParserDestroy(parser);
    return 0;
};
