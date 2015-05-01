%{
 
#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"

extern int yyerror(YYLTYPE *yylloc, void *scanner, Expression **expression, const char *message);
 
%}

%output = "Parser.c"
%defines
 
%pure-parser
%lex-param   { void *scanner }
%parse-param { void *scanner }
%parse-param { Expression **expression }
%locations

%union {
    int i;
    float f;
    char *s;
    Expression *expression;
}

%token TOKEN_BEGIN

%token <i>INTEGER
%token <i>SIGNED_INT
%token <f>FLOAT
%token <s>STRING

%token <s>NOTE
%token <s>KEY_SIGN
%token <s>QUANTIZE
%token <s>OCTAVE_SHIFT

%token TIME
%token TEMPO
%token MARKER
%token CHANNEL
%token VELOCITY
%token GATETIME
%token OCTAVE

%token REST
%token TIE

%token LENGTH
%token OFFSET
%token LOCATION

%token PLUS
%token MINUS
%token MULTIPLY
%token DIVISION
%token ASSIGN

%token COMMA

%token LCURLY
%token RCURLY

%token <s>IDENTIFIER

%token EOL

%token TOKEN_END

%type <expression> statement_list
%type <expression> statement

%%
 
input
    : statement_list { *expression = $1; }
    ;

statement_list
    : statement
    | statement_list statement { $$ = ExpressionAddRight($1, $2); }
    ;

statement
    : note
    ;

note
    : NOTE { $$ = ExpressionCreateStringValue(&@$, ExpressionTypeNote, $1); }
    ;

%%
