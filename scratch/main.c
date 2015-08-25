#include "Parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_HISTORY 10

typedef struct _Sequence {
    StatementHandler handler;
} Sequence;

static bool SequenceProcess(void *self, ParseLocation *location, StatementType type, ...)
{
    printf("%d: %s\n", location->line, StatementType2String(type));
    return true;
}

static void SequenceError(void *self, ParseLocation *location, ParseError error)
{
    printf("%d:%d %s\n", location->line, location->column, ParseError2String(error));
}

int main(int argc, char **argv)
{
    const char *prompt = "namidi> ";

    char *line = NULL;
    int historyCount = 0;
    Sequence sequence = {SequenceProcess, SequenceError};
    Parser *parser = ParserCreate(SyntaxNAMidi, &sequence);

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
