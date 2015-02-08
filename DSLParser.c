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

bool DSLParserParseFile(const char *filepath, Expression **expression, ParseError *error)
{
    bool ret = false;
    void *scanner;
    *expression = NULL;

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

    if (yyparse(scanner, expression)) {
        error->kind = PARSE_ERROR_SYNTAX_ERROR;
        if (*expression) {
            deleteExpression(*expression);
            *expression = NULL;
        }
        goto ERROR_2;
    }

    error->kind = PARSE_ERROR_NOERROR;
    ret = true;

ERROR_2:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
ERROR_1:
    fclose(fp);

    return ret;
}
