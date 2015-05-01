#include "NAMidiParser.h"

#include "Parser.h"
#include "Lexer.h"

#include <stdlib.h>


static bool parseDSL(const char *filepath, Expression **expression, ParseError **error);

struct _NAMidiParser {
    NAMidiParserCallbacks *callbacks;
};

NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->callbacks = callbacks;
    return self;
}

void NAMidiParserDestroy(NAMidiParser *self)
{
    free(self);
}

void NAMidiParserExecuteParse(NAMidiParser *self)
{
    Expression *expression;
    *error = NULL;

    if (!parseDSL(filepath, &expression, error)) {
        return NULL;
    }

#if 0
    dumpExpression(expression);
#endif
}

static bool parseDSL(const char *filepath, Expression **expression, ParseError **error)
{
    bool ret = false;
    void *scanner;
    *expression = NULL;

    *error = ParseErrorCreate();
    (*error)->filepath = filepath;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        error->kind = PARSE_ERROR_FILE_NOT_FOUND;
        return ret;
    }

    if (yylex_init_extra(*error, &scanner)) {
        (*error)->kind = PARSE_ERROR_INIT_ERROR;
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, expression)) {
        (*error)->kind = PARSE_ERROR_SYNTAX_ERROR;
        if (*expression) {
            deleteExpression(*expression);
            *expression = NULL;
        }
        goto ERROR_2;
    }

    free(*error);
    *error = NULL;
    ret = true;

ERROR_2:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
ERROR_1:
    fclose(fp);

    return ret;
}
