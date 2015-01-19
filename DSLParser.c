#include "DSLParser.h"
#include "Parser.h"
#include "Lexer.h"

#include <stdio.h>
#include <stdlib.h>

int yyerror(YYLTYPE *yylloc, void *scanner, Expression **expression, const char *message)
{
    DSLParser *self = yyget_extra(scanner);
    Location *location = (Location *)yylloc;

    printf("yyerror() %d %d %d %d %s\n", location->firstLine, location->firstColumn, location->lastLine, location->lastColumn, message);
    return 0;
}

DSLParser *DSLParserCreate()
{
    return (DSLParser *)calloc(1, sizeof(DSLParser));
}

bool DSLParserParseFile(DSLParser *self, const char *filepath)
{
    void *scanner;
    bool ret = false;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        self->error = DSLPARSER_FILE_NOT_FOUND;
        return ret;
    }

    if (yylex_init_extra(&self, &scanner)) {
        self->error = DSLPARSER_INIT_ERROR;
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, &self->expression)) {
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

void DSLParserDumpExpression(DSLParser *self)
{
    dumpExpression(self->expression);
}

void DSLParserDestroy(DSLParser *self)
{
    deleteExpression(self->expression);
    free(self);
}
