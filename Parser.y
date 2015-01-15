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
%token CUTOFF
%token NOTE

%token STEP
%token FROM
%token TO
%token REPLACE
%token MIX
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

%token <s>IDENTIFIER

%token EOL

%token PARAM
%token SOUND_SELECT
%token GATETIME_CUTOFF
%token NOTE_BLOCK
%token NOTE_NO_LIST

%type <expression> statement_list
%type <expression> statement
%type <expression> title
%type <expression> resolution
%type <expression> time
%type <expression> time_param_list
%type <expression> time_param
%type <expression> tempo
%type <expression> tempo_param_list
%type <expression> tempo_param
%type <expression> marker
%type <expression> marker_param_list
%type <expression> marker_param
%type <expression> velocity
%type <expression> gatetime
%type <expression> channel
%type <expression> sound_select
%type <expression> sound_select_param_list
%type <expression> sound_select_param
%type <expression> note
%type <expression> note_param_list
%type <expression> note_param
%type <expression> string
%type <expression> integer
%type <expression> float
%type <expression> division
%type <expression> from
%type <expression> from_param
%type <expression> step
%type <expression> step_param
%type <expression> integer_list
%type <expression> integer_list_param
%type <expression> note_block
%type <expression> note_block_param_list
%type <expression> note_block_param
%type <expression> note_no_list
%type <expression> note_no_list_param

%%
 
input
    : statement_list { *expression = $1; }
    ;

statement_list
    : statement                { $$ = $1; }
    | statement_list statement { $$ = addRightExpression($1, $2); }
    ;

statement
    : title                    { $$ = $1; }
    | resolution               { $$ = $1; }
    | time                     { $$ = $1; }
    | tempo                    { $$ = $1; }
    | marker                   { $$ = $1; }
    | velocity                 { $$ = $1; }
    | gatetime                 { $$ = $1; }
    | channel                  { $$ = $1; }
    | sound_select             { $$ = $1; }
    | note                     { $$ = $1; }
    ;

title
    : TITLE string             { $$ = createExpression(TITLE, $2, NULL); }
    ;

resolution
    : RESOLUTION integer       { $$ = createExpression(RESOLUTION, $2, NULL); }
    ;

time
    : TIME time_param_list     { $$ = createExpression(TIME, $2, NULL); }
    ;
time_param_list
    : time_param                  { $$ = $1; }
    | time_param_list time_param  { $$ = addRightExpression($1, $2); }
    ;
time_param
    : division                    { $$ = $1; }
    | from                        { $$ = $1; }
    ;

tempo
    : TEMPO tempo_param_list     { $$ = createExpression(TEMPO, $2, NULL); }
    ;
tempo_param_list
    : tempo_param                  { $$ = $1; }
    | tempo_param_list tempo_param { $$ = addRightExpression($1, $2); }
    ;
tempo_param
    : integer                    { $$ = $1; }
    | float                      { $$ = $1; }
    | from                       { $$ = $1; }
    ;

marker
    : MARKER marker_param_list    { $$ = createExpression(MARKER, $2, NULL); }
    ;
marker_param_list
    : marker_param                     { $$ = $1; }
    | marker_param_list marker_param   { $$ = addRightExpression($1, $2); }
    ;
marker_param
    : string  { $$ = $1; }
    | from    { $$ = $1; }
    ;

velocity
    : VELOCITY integer { $$ = createExpression(VELOCITY, $2, NULL); }
    ;

gatetime
    : GATETIME integer         { $$ = createExpression(GATETIME, $2, NULL); }
    | GATETIME CUTOFF integer  { $$ = createExpression(GATETIME_CUTOFF, $3, NULL); }
    ;

channel
    : CHANNEL integer { $$ = createExpression(CHANNEL, $2, NULL); }
    ;

sound_select
    : SOUND SELECT sound_select_param_list { $$ = createExpression(SOUND_SELECT, $3, NULL); }
    ;
sound_select_param_list
    : sound_select_param                          { $$ = $1; }
    | sound_select_param_list sound_select_param  { $$ = addRightExpression($1, $2); }
    ;
sound_select_param
    : integer_list { $$ = $1; }
    | from         { $$ = $1; }
    ;

note
    : NOTE note_param_list { $$ = createExpression(NOTE, $2, NULL); }
    ;
note_param_list
    : note_param                 { $$ = $1; }
    | note_param_list note_param { $$ = addRightExpression($1, $2); }
    ;
note_param
    : from
    | step
    | note_block
    ;

string
    : STRING  { $$ = createExpression(PARAM, createStringValue(STRING, $1), NULL); }
    ;
integer
    : INTEGER  { $$ = createExpression(PARAM, createIntegerValue(INTEGER, $1), NULL); }
    ;
float
    : FLOAT  { $$ = createExpression(PARAM, createFloatValue(FLOAT, $1), NULL); }
    ;

division
    : INTEGER DIVISION INTEGER { $$ = createExpression(PARAM,
                                        createExpression(DIVISION,
                                          createIntegerValue(INTEGER, $1),
                                          createIntegerValue(INTEGER, $3)
                                        ),
                                        NULL
                                      ); }
    ;

from
    : FROM from_param { $$ = createExpression(PARAM, createExpression(FROM, $2, NULL), NULL); }
    ;
from_param
    : INTEGER  { $$ = createIntegerValue(INTEGER, $1); }
    | LOCATION { $$ = createStringValue(LOCATION, $1); }
    ;

step
    : STEP step_param { $$ = createExpression(PARAM, createExpression(STEP, $2, NULL), NULL); }
    ;
step_param
    : INTEGER  { $$ = createIntegerValue(INTEGER, $1); }
    ;

integer_list
    : integer_list_param { $$ = createExpression(PARAM, $1, NULL); }
    ;

integer_list_param
    : INTEGER                     { $$ = createIntegerValue(INTEGER, $1); }
    | integer_list_param INTEGER  { $$ = addRightExpression($1, createIntegerValue(INTEGER, $2)); }
    ;

note_block
    : LCURLY note_block_param_list RCURLY { $$ = createExpression(PARAM, createExpression(NOTE_BLOCK, $2, NULL), NULL); }
    ;
note_block_param_list
    : note_block_param                       { $$ = $1; }
    | note_block_param_list note_block_param { $$ = addRightExpression($1, $2); }
    ;
note_block_param
    : NOTE_NO { $$ = createStringValue(NOTE_NO, $1); }
    | REST    { $$ = createExpression(REST, NULL, NULL); }
    | TIE     { $$ = createExpression(TIE, NULL, NULL); }
    | note_no_list { $$ = $1; }
    ;

note_no_list
    : note_no_list_param { $$ = createExpression(NOTE_NO_LIST, $1, NULL); }
    ;
note_no_list_param
    : NOTE_NO COMMA              { $$ = createStringValue(NOTE_NO, $1); }
    | note_no_list_param NOTE_NO { $$ = addRightExpression($1, createStringValue(NOTE_NO, $2)); }
    ;

%%
