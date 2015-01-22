#include "DSLParser.h"
#include "Parser.h"
#include "Lexer.h"

int yyerror(YYLTYPE *yylloc, void *scanner, Expression **expression, const char *message)
{
    DSLParserError *error = yyget_extra(scanner);
    error->location = *((Location *)yylloc);
    error->message = message;
    return 0;
}

Expression *DSLParserParseFile(const char *filepath, DSLParserError *error)
{
    void *scanner;
    Expression *ret = NULL;

    error->filepath = filepath;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        error->kind = DSLPARSER_FILE_NOT_FOUND;
        return ret;
    }

    if (yylex_init_extra(error, &scanner)) {
        error->kind = DSLPARSER_INIT_ERROR;
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, &ret)) {
        error->kind = DSLPARSER_PARSE_ERROR;
        if (ret) {
            deleteExpression(ret);
            ret = NULL;
        }
        goto ERROR_2;
    }

ERROR_2:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
ERROR_1:
    fclose(fp);

    return ret;
}

void DSLParserDumpExpression(Expression *expression)
{
    dumpExpression(expression);
}

void DSLParserDeleteExpression(Expression *expression)
{
    deleteExpression(expression);
}
