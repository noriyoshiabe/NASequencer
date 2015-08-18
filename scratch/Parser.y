%{
 
#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"

extern int yyerror(YYLTYPE *yylloc, void *scanner, ParserCallback callback, ParserErrorCallback errorCallback, const char *message);
 
%}

%output = "Parser.c"
%defines
 
%pure-parser
%lex-param   { void *scanner }
%parse-param { void *scanner }
%parse-param { ParserCallback callback }
%parse-param { ParserErrorCallback errorCallback }
%locations

%union {
    int i;
    float f;
    char *s;
}

%token <i>INTEGER
%token <f>FLOAT
%token <s>STRING

%token RESOLUTION

%token PLUS
%token MINUS
%token ASSIGN
%token COMMA

%token <s>IDENTIFIER

%token EOL

%%
 
input
    :
    | statement { return 1; }
    | EOL { return 1; }
    ;

statement
    : RESOLUTION INTEGER { callback(yyget_extra(scanner), RESOLUTION, $2); }
    ;

%%

int yyerror(YYLTYPE *yylloc, void *scanner, ParserCallback callback, ParserErrorCallback errorCallback, const char *message)
{
    errorCallback(yyget_extra(scanner), yylloc->first_line, yylloc->first_column, message);
    return 0;
}
