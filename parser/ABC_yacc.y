%{
 
#include "ABC_yacc.h"
#include "ABC_lex.h"
#include "ABCParser.h"

#include <ctype.h>

extern int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *message);

static bool callProcess(YYLTYPE *yylloc, yyscan_t scanner, ABCExpressionType type, ...);
static void postProcess(YYLTYPE *yylloc, yyscan_t scanner, ABCExpressionType type, ...);

#define Process(yylloc, type, ...) \
    bool success = callProcess(yylloc, scanner, type, __VA_ARGS__); \
    postProcess(yylloc, scanner, type, __VA_ARGS__); \
    if (!success) { \
        return 1;\
    }

%}

%name-prefix = "ABC_"
%output = "ABC_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%locations

%union {
    int i;
    float f;
    char *s;
}

%token <s>NOTE

%%

input
    :
    | expr_list
    ;

expr_list
    : expr
    | expr_list expr
    ;

expr
    : NOTE { Process(&@$, ABCExpressionTypeNote, $1); }
    ;

%%

int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *message)
{
    ABCParser *parser = ABC_get_extra(scanner);
    ABCParserError(parser, yylloc->first_line, yylloc->first_column, ParseErrorKindSyntaxError);
    return 0;
}

static bool callProcess(YYLTYPE *yylloc, yyscan_t scanner, ABCExpressionType type, ...)
{
    ABCParser *parser = ABC_get_extra(scanner);

    va_list argList;
    va_start(argList, type);
    bool success = ABCParserProcess(parser, yylloc->first_line, yylloc->first_column, type, argList);
    va_end(argList);
    return success;
}

static void postProcess(YYLTYPE *yylloc, yyscan_t scanner, ABCExpressionType type, ...)
{
    va_list argList;

    switch (type) {
    case ABCExpressionTypeNote:
        va_start(argList, type);
        free(va_arg(argList, char *));
        va_end(argList);
        break;
    default:
        break;
    }
}
