#include "DSLParser.h"
#include "Parser.h"
#include "Lexer.h"
#include <NACFHelper.h>

int yyparse(void *scanner, Expression **expression);

int yyerror(YYLTYPE *yylloc, void *scanner, Expression **expression, const char *message)
{
    ParseError *error = yyget_extra(scanner);
    error->location = *((ParseLocation *)yylloc);
    error->message = CFStringCreateWithCString(NULL, message, kCFStringEncodingUTF8);
    return 0;
}

bool DSLParserParseFile(CFStringRef filepath, Expression **expression, ParseError **error)
{
    bool ret = false;
    void *scanner;
    *expression = NULL;

    *error = NATypeNew(ParseError);
    (*error)->filepath = CFRetain(filepath);

    FILE *fp = fopen(NACFString2CString(filepath), "r");
    if (!fp) {
        (*error)->kind = PARSE_ERROR_FILE_NOT_FOUND;
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

    NARelease(*error);
    *error = NULL;
    ret = true;

ERROR_2:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
ERROR_1:
    fclose(fp);

    return ret;
}
