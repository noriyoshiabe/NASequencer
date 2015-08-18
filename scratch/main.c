#include <stdio.h>

#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"

int yyparse(void *scanner, ParserCallback callback, ParserErrorCallback errorCallback);

void __ParserCallback(void *context, int statement, ...)
{
    int *_context = context;
    printf("c=%d statment=%d\n", *_context, statement);
}

void __ParserErrorCallback(void *context, int line, int column, const char *message)
{
    int *_context = context;
    printf("c=%d line=%d column=%d %s\n", *_context, line, column, message);
}


int main(int argc, char **argv)
{
    yyscan_t scanner;

    int context = 100;

    for (;;) {
        yylex_init_extra(&context, &scanner);

        printf("namidi > ");
        fflush(stdout);

        char buffer[1024];
        fgets(buffer, 1024, stdin);
        yy_scan_buffer(buffer, 1024, scanner);
        yyparse(scanner, __ParserCallback, __ParserErrorCallback);

        yylex_destroy(scanner);
    }

    return 0;
}
