%{
 
#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"
#include <stdio.h>
 
int yyerror(YYLTYPE *location, Expression **expression, yyscan_t scanner, const char *msg) {
printf("############ %d %d %s\n", location->last_line, location->last_column, msg);
    // Add error handling routine as needed
    return 0;
}
 
%}
 
%code requires {
 
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif
 
}
 
%output  "Parser.c"
%defines "Parser.h"
 
%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { Expression **expression }
%parse-param { yyscan_t scanner }
%locations

%union {
    int i;
    float f;
    char *s;
    Expression *expression;
}

%type <expression> expr
 
%token <i>INTEGER
%token <f>FLOAT
%token <s>STRING

%token <s>NOTE_NO
%token <s>LOCATION
%token <s>MB_LENGTH

%token RESOLUTION
%token TITLE
%token TIME
%token TEMPO
%token MARKER
%token SOUND
%token SELECT
%token CHANNEL
%token VELOCITY
%token GATETIME
%token NOTE

%token STEP
%token CUTOFF
%token OFFSET
%token LENGTH

%token REST
%token TIE

%token PLUS
%token MINUS
%token DIVISION
%token MULTIPLY
%token ASSIGN

%token SEMICOLON
%token COMMA

%token LPAREN
%token RPAREN
%token LCURLY
%token RCURLY

%token IDENTIFIER

%token EOL

%left ASSIGN
%left LPAREN
%left RPAREN
%left LCURLY
%left RCURLY

%token SOUND_SELECT
%token GATETIME_CUTOFF
%token NEGATIVE
%token NEGATIVE_FLOAT


%%
 
input
    : expr { *expression = $1; }
    ;
 
expr
    : expr expr               { $$ = addRightExpression($1, $2); }

    | INTEGER                 { $$ = createIntegerValue(INTEGER, $1); }
    | PLUS INTEGER            { $$ = createIntegerValue(INTEGER, $2); }
    | MINUS INTEGER           { $$ = createIntegerValue(NEGATIVE, $2); }
    | FLOAT                   { $$ = createFloatValue(FLOAT, $1); }
    | PLUS FLOAT              { $$ = createFloatValue(FLOAT, $2); }
    | MINUS FLOAT             { $$ = createFloatValue(NEGATIVE_FLOAT, $2); }
    | STRING                  { $$ = createStringValue(STRING, $1); }

    | NOTE_NO                 { $$ = createStringValue(NOTE_NO, $1); }
    | LOCATION                { $$ = createStringValue(LOCATION, $1); }
    | MB_LENGTH               { $$ = createStringValue(MB_LENGTH, $1); }

    | RESOLUTION expr         { $$ = createExpression(RESOLUTION, $1, NULL); }
    | TITLE expr              { $$ = createExpression(TITLE, $1, NULL); }
    | TIME expr               { $$ = createExpression(TIME, $1, NULL); }
    | TEMPO expr              { $$ = createExpression(TEMPO, $1, NULL); }
    | MARKER expr             { $$ = createExpression(MARKER, $1, NULL); }
    | SOUND SELECT expr       { $$ = createExpression(SOUND_SELECT, $2, NULL); }
    | CHANNEL expr            { $$ = createExpression(CHANNEL, $1, NULL); }
    | VELOCITY expr           { $$ = createExpression(VELOCITY, $1, NULL); }
    | GATETIME expr           { $$ = createExpression(GATETIME, $1, NULL); }
    | GATETIME CUTOFF expr    { $$ = createExpression(GATETIME_CUTOFF, $2, NULL); }
    | NOTE expr               { $$ = createExpression(NOTE, $1, NULL); }

    | STEP expr               { $$ = createExpression(STEP, $1, NULL); }
    | FROM expr               { $$ = createExpression(FROM, $1, NULL); }
    | TO expr                 { $$ = createExpression(TO, $1, NULL); }
    | REPLACE expr            { $$ = createExpression(REPLACE, $1, NULL); }
    | MIX expr                { $$ = createExpression(MIX, $1, NULL); }
    | OFFSET expr             { $$ = createExpression(OFFSET, $1, NULL); }
    | LENGTH expr             { $$ = createExpression(LENGTH, $1, NULL); }

    | REST                    { $$ = createExpression(REST, NULL, NULL); }
    | TIE                     { $$ = createExpression(TIE, NULL, NULL); }

    | expr PLUS expr          { $$ = createExpression(PLUS, $1, $3); }
    | expr MINUS expr         { $$ = createExpression(MINUS, $1, $3); }
    | expr MULTIPLY expr      { $$ = createExpression(MULTIPLY, $1, $3); }
    | expr DIVISION expr      { $$ = createExpression(DIVISION, $1, $3); }
    | expr ASSIGN expr        { $$ = createExpression(ASSIGN, $1, $3); }
    | expr COMMA expr         { $$ = addRightExpression($1, $3); }

    | LPAREN expr RPAREN      { $$ = $2; }
    | LCURLY expr RCURLY      { $$ = $2; }
    
    | IDENTIFIER              { $$ = createStringValue(IDENTIFIER, $1); }
    ;

%%
