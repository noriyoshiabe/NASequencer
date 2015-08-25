#include "Parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_HISTORY_NO 10

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
    int index = 0;
    int history_no = 0;
    HIST_ENTRY *history = NULL;

    Sequence sequence = {SequenceProcess, SequenceError};

    Parser *parser = ParserCreate(SyntaxNAMidi, &sequence);

    while ((line = readline(prompt))) {
        ParserScanBuffer(parser, line, strlen(line));
        add_history(line);

        if (++history_no > MAX_HISTORY_NO) {
            history = remove_history(0);
            free(history);
        }

        free(line);
    }

    printf("\n");

    ParserDestroy(parser);

    clear_history();
    free(line);

    return 0;
}
