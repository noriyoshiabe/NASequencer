#include "DSLParser.h"
#include "Parser.h"
#include "Lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yyerror(YYLTYPE *yylloc, void *scanner, Expression **expression, const char *message)
{
    DSLParser *self = yyget_extra(scanner);
    self->location = *((Location *)yylloc);
    self->message = strdup(message);
    return 0;
}

DSLParser *DSLParserCreate()
{
    return (DSLParser *)calloc(1, sizeof(DSLParser));
}

bool DSLParserParseFile(DSLParser *self, const char *filepath, Expression **expression)
{
    void *scanner;
    bool ret = false;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        self->error = DSLPARSER_FILE_NOT_FOUND;
        return ret;
    }

    if (yylex_init_extra(self, &scanner)) {
        self->error = DSLPARSER_INIT_ERROR;
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, expression)) {
        self->error = DSLPARSER_PARSE_ERROR;
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

void DSLParserDestroy(DSLParser *self)
{
    if (self->message) {
        free(self->message);
    }
    free(self);
}

void DSLParserDumpExpression(Expression *expression)
{
    dumpExpression(expression);
}

void DSLParserDeleteExpression(Expression *expression)
{
    deleteExpression(expression);
}

