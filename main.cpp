#include "parser.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    try {
        Sequence *sequence = Parser::parse("./test.namidi");
    } catch (Parser::Exception &e) {
        printf("%s\n", e.what());
    }
    return 0;
}
