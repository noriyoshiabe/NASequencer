#include "parser.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    ParseContext *context = Parser::parse("./test.namidi");
    for (std::string *error : context->errors) {
        printf("%s\n", error->c_str());
    }
    for (std::string *warning : context->warnings) {
        printf("%s\n", warning->c_str());
    }
    return 0;
}
