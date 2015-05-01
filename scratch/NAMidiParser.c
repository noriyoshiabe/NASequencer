#include "NAMidiParser.h"
#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"

#include <stdlib.h>

struct _NAMidiParser {
    NAMidiParserCallbacks *callbacks;
    void *context;
};

int yyparse(void *scanner, Expression **expression);
static bool parseDSL(NAMidiParser *self, const char *filepath, Expression **expression);

NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks, void *context)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->callbacks = callbacks;
    self->context = context;
    return self;
}

void NAMidiParserDestroy(NAMidiParser *self)
{
    free(self);
}

bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath)
{
    Expression *expression;

    if (!parseDSL(self, filepath, &expression)) {
        return false;
    }

#if 1
    ExpressionDump(expression);
#endif

    return true;
}

static bool parseDSL(NAMidiParser *self, const char *filepath, Expression **expression)
{
    bool ret = false;
    void *scanner;
    *expression = NULL;

    ParseLocation location;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        self->callbacks->onError(self->context, filepath, 0, 0, ParseErrorFileNotFound);
        return ret;
    }

    if (yylex_init_extra(&location, &scanner)) {
        self->callbacks->onError(self->context, filepath, 0, 0, ParseErrorInitError);
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, expression)) {
        self->callbacks->onError(self->context, filepath, location.firstLine, location.firstColumn, ParseErrorSyntaxError);

        if (*expression) {
            ExpressionDestroy(*expression);
            *expression = NULL;
        }
        goto ERROR_2;
    }

    ret = true;

ERROR_2:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
ERROR_1:
    fclose(fp);

    return ret;
}

int yyerror(YYLTYPE *yylloc, void *scanner, Expression **expression, const char *message)
{
    ParseLocation *location = yyget_extra(scanner);
    *location = *((ParseLocation *)yylloc);
    return 0;
}
