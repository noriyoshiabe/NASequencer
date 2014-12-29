#include "parser.h"

int main(int argc, char **argv)
{
    Parser *parser = new Parser("./test.namidi");
    parser->parse(new Context());
}
