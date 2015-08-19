%{
 
#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"

extern int yyerror(YYLTYPE *yylloc, void *scanner, ParserCallback callback, ParserErrorCallback errorCallback, const char *message);
 
#define CALLBACK(location, statement, ...) callback(yyget_extra(scanner), (ParseLocation *)location, statement, __VA_ARGS__)

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
%token TITLE

%token PLUS
%token MINUS
%token ASSIGN
%token COMMA

%token <s>IDENTIFIER

%token EOL

%%
 
input
    :
    | statement
    | EOL
    ;

statement
    : RESOLUTION INTEGER { CALLBACK(&@$, StatementResolution, $2); }
    | TITLE STRING { CALLBACK(&@$, StatementTitle, $2); }
    ;

%%

int yyerror(YYLTYPE *yylloc, void *scanner, ParserCallback callback, ParserErrorCallback errorCallback, const char *message)
{
    errorCallback(yyget_extra(scanner), (ParseLocation *)yylloc, message);
    return 0;
}
