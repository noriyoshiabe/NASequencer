%{
 
#include "ASTNode.h"
#include "Parser.h"
#include "Lexer.h"
#include <stdio.h>
 
int yyerror(YYLTYPE *location, ASTNode **astRoot, yyscan_t scanner, const char *msg) {
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
%define api.value.type {ASTNode}
%lex-param   { yyscan_t scanner }
%parse-param { ASTNode **astRoot }
%parse-param { yyscan_t scanner }
%locations
 
%token INTEGER
%token FLOAT
%token STRING

%token NOTE_NO
%token LOCATION
%token MEASURE
%token BEAT

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
%token DIVISION
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

%token TIME_SIGN
%token SOUND_SELECT
%token SOUND_PARAM
%token GATETIME_CUTOFF


%%
 
input
    : statement_list { *astRoot = $1; }
    ;

statement_list
    : statement                { $$ = $1; }
    | statement_list statement { $$ = addSiblingToNode($1, $2); }
    ;

statement
    : RESOLUTION integer { $$ = createNodeWithChild(RESOLUTION, $2); }
    | TITLE string       { $$ = createNodeWithChild(TITLE, $2); }
    | TIME time_sign     { $$ = createNodeWithChild(TIME, $2); }
    | TEMPO float        { $$ = createNodeWithChild(TEMPO, $2); }
    | TEMPO integer      { $$ = createNodeWithChild(TEMPO, $2); }
    | MARKER string      { $$ = createNodeWithChild(MARKER, $2); }
    | SOUND SELECT sound_param
                         { $$ = createNodeWithChild(SOUND_SELECT, $3); }
    | CHANNEL integer    { $$ = createNodeWithChild(CHANNEL, $2); }
    | VELOCITY integer   { $$ = createNodeWithChild(VELOCITY, $2); }
    | GATETIME integer   { $$ = createNodeWithChild(GATETIME, $2); }
    | GATETIME CUTOFF integer
                         { $$ = createNodeWithChild(GATETIME_CUTOFF, $2); }
    | note
    | rest
    | tie
    ;


param_list
    : param
    | param_list param
    ;

param
    : integer
    | float
    | string
    | time_sign
    | sound_param
    | from
    | to
    | step
    | block
    ;



integer
    : INTEGER { $$ = createNodeWithInteger(INTEGER, yytext); }
    ;

float
    : FLOAT { $$ = createNodeWithFloat(FLOAT, yytext); }
    ;

string
    : STRING { $$ = createNodeWithString(STRING, yytext); }
    ;

location
    : LOCATION { $$ = createNodeWithString(LOCATION, yytext); }
    | INTEGER  { $$ = createNodeWithString(INTEGER, yytext); }
    | MEASURE  { $$ = createNodeWithString(MEASURE, yytext); }
    | BEAT     { $$ = createNodeWithString(BEAT, yytext); }
    ;

time_sign
    : integer DIVISION integer { $$ = createNodeWithChildren(TIME_SIGN, $2, $4, NULL); }
    ;

sound_param
    : integer integer integer  { $$ = createNodeWithChildren(SOUND_PARAM, $1, $2, $3, NULL); }

from
    : FROM location { $$ = createNodeWithChild(FROM, $2); }
    ;

to
    : FROM location { $$ = createNodeWithChild(FROM, $2); }
    ;

step
    : STEP integer { $$ = createNodeWithChild(FROM, $2); }

eos
    : EOL
    | SEMICOLON
    ;

line
    : eos
    | statement eos
    | line eos
    ;

statement
    : resolution { create }


input
    : expr { *expression = $1; }
    ;
 
expr
    : expr[L] TOKEN_PLUS expr[R] { $$ = createOperation( ePLUS, $L, $R ); }
    | expr[L] TOKEN_MULTIPLY expr[R] { $$ = createOperation( eMULTIPLY, $L, $R ); }
    | TOKEN_LPAREN expr[E] TOKEN_RPAREN { $$ = $E; }
    | TOKEN_NUMBER { $$ = createNumber($1);
    printf("%d %d %d %d\n", @1.first_column, @1.first_line, @1.last_column, @1.last_line);
    }
    ;
 
%%
