#include "DSLParser.h"
#include "Parser.h"
#include "Lexer.h"

int yyerror(YYLTYPE *yylloc, void *scanner, Expression **expression, const char *message)
{
    ParseError *error = yyget_extra(scanner);
    error->location = *((Location *)yylloc);
    error->message = message;
    return 0;
}

Expression *DSLParserParseFile(const char *filepath, ParseError *error)
{
    void *scanner;
    Expression *ret = NULL;

    error->filepath = filepath;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        error->kind = PARSE_ERROR_FILE_NOT_FOUND;
        return ret;
    }

    if (yylex_init_extra(error, &scanner)) {
        error->kind = PARSE_ERROR_INIT_ERROR;
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, &ret)) {
        error->kind = PARSE_ERROR_SYNTAX_ERROR;
        if (ret) {
            deleteExpression(ret);
            ret = NULL;
        }
        goto ERROR_2;
    }

    error->kind = PARSE_ERROR_NOERROR;

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
