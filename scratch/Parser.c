#include "Parser.h"

#include <stdlib.h>

struct _Parser {
    void *scanner;
};

Parser *ParserCreate(Syntax syntax, void *handler)
{
    Parser *self = calloc(1, sizeof(Parser));
    return self;
}

void ParserDestroy(Parser *self)
{
    free(self);
}

void ParserScanBuffer(Parser *self, char *buffer, int length)
{
}
