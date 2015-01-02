#include "parser.h"
#include "console_writer.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    ParseContext *context = Parser::parse("./test.namidi");
    ConsoleWriter writer;
    writer.write(context);

    delete context;
    return 0;
}
