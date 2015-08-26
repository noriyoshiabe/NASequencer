#include "Parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_HISTORY 10

static bool Process(void *receiver, ParseContext *context, StatementType type, va_list argList)
{
    printf("%s: %d: %s\n", context->extra, context->location.line, StatementType2String(type));
    return true;
}

static void Error(void *receiver, ParseContext *context, ParseError error)
{
    printf("%s: %d:%d %s\n", context->extra, context->location.line, context->location.column, ParseError2String(error));
}

int main(int argc, char **argv)
{
    const char *prompt = "namidi> ";

    char *line = NULL;
    int historyCount = 0;

    int receiver = 1;
    StatementHandler handler = {Process, Error};
    Parser *parser = ParserCreate(SyntaxNAMidi, &handler, &receiver, "hoge");

    while ((line = readline(prompt))) {
        ParserScanString(parser, line);

        add_history(line);
        free(line);

        if (MAX_HISTORY < ++historyCount) {
            free(remove_history(0));
        }
    }

    printf("\n");

    ParserDestroy(parser);
    clear_history();

    return 0;
}
