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

%token TIME_SIGN
%token SOUND_SELECT
%token INTEGER_LIST
%token GATETIME_CUTOFF
%token NOTE_BLOCK
%token NOTE_NO_LIST
%token PATTERN_DEFINE
%token PATTERN_BLOCK

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
%type <expression> pattern_define
%type <expression> pattern_block
%type <expression> pattern_statement_list
%type <expression> pattern_statement
%type <expression> string
%type <expression> integer
%type <expression> float
%type <expression> time_sign
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
%type <expression> note_no
%type <expression> identifier

%%
 
input
    : statement_list { *expression = $1; }
    ;

statement_list
    : statement
    | statement_list statement { $$ = addRightExpression($1, $2); }
    ;

statement
    : title
    | resolution
    | time
    | tempo
    | marker
    | velocity
    | gatetime
    | channel
    | sound_select
    | note
    | pattern_define
    ;

title
    : TITLE string { $$ = createExpression(TITLE, $2, NULL); }
    ;

resolution
    : RESOLUTION integer { $$ = createExpression(RESOLUTION, $2, NULL); }
    ;

time
    : TIME time_param_list { $$ = createExpression(TIME, $2, NULL); }
    ;
time_param_list
    : time_param
    | time_param_list time_param { $$ = addRightExpression($1, $2); }
    ;
time_param
    : time_sign
    | from
    ;

tempo
    : TEMPO tempo_param_list { $$ = createExpression(TEMPO, $2, NULL); }
    ;
tempo_param_list
    : tempo_param
    | tempo_param_list tempo_param { $$ = addRightExpression($1, $2); }
    ;
tempo_param
    : integer
    | float
    | from
    ;

marker
    : MARKER marker_param_list { $$ = createExpression(MARKER, $2, NULL); }
    ;
marker_param_list
    : marker_param
    | marker_param_list marker_param { $$ = addRightExpression($1, $2); }
    ;
marker_param
    : string
    | from
    ;

velocity
    : VELOCITY integer { $$ = createExpression(VELOCITY, $2, NULL); }
    ;

gatetime
    : GATETIME integer { $$ = createExpression(GATETIME, $2, NULL); }
    | GATETIME CUTOFF integer { $$ = createExpression(GATETIME_CUTOFF, $3, NULL); }
    ;

channel
    : CHANNEL integer { $$ = createExpression(CHANNEL, $2, NULL); }
    ;

sound_select
    : SOUND SELECT sound_select_param_list { $$ = createExpression(SOUND_SELECT, $3, NULL); }
    ;
sound_select_param_list
    : sound_select_param
    | sound_select_param_list sound_select_param { $$ = addRightExpression($1, $2); }
    ;
sound_select_param
    : integer_list
    | from
    ;

note
    : NOTE note_param_list { $$ = createExpression(NOTE, $2, NULL); }
    ;
note_param_list
    : note_param
    | note_param_list note_param { $$ = addRightExpression($1, $2); }
    ;
note_param
    : from
    | step
    | note_block
    ;

pattern_define
    : identifier ASSIGN pattern_block { $$ = createExpression(PATTERN_DEFINE, addRightExpression($1, $3), NULL); }
    ;
pattern_block
    : LCURLY pattern_statement_list RCURLY { $$ = createExpression(PATTERN_BLOCK, $2, NULL); }
    ;

pattern_statement_list
    : pattern_statement
    | pattern_statement_list pattern_statement { $$ = addRightExpression($1, $2); }
    ;
pattern_statement
    : time
    | tempo
    | marker
    | velocity
    | gatetime
    | channel
    | sound_select
    | note
    | pattern_define
    ;

string
    : STRING { $$ = createStringValue(STRING, $1); }
    ;
integer
    : INTEGER { $$ = createIntegerValue(INTEGER, $1); }
    ;
float
    : FLOAT { $$ = createFloatValue(FLOAT, $1); }
    ;

time_sign
    : integer DIVISION integer { $$ = createExpression(TIME_SIGN, addRightExpression($1, $3), NULL); }
    ;

from
    : FROM from_param { $$ = createExpression(FROM, $2, NULL); }
    ;
from_param
    : integer
    | LOCATION { $$ = createStringValue(LOCATION, $1); }
    ;

step
    : STEP step_param { $$ = createExpression(STEP, $2, NULL); }
    ;
step_param
    : integer
    ;

integer_list
    : integer_list_param { $$ = createExpression(INTEGER_LIST, $1, NULL); }
    ;

integer_list_param
    : integer
    | integer_list_param integer { $$ = addRightExpression($1, $2); }
    ;

note_block
    : LCURLY note_block_param_list RCURLY { $$ = createExpression(NOTE_BLOCK, $2, NULL); }
    ;
note_block_param_list
    : note_block_param
    | note_block_param_list note_block_param { $$ = addRightExpression($1, $2); }
    ;
note_block_param
    : note_no
    | note_no_list
    | REST { $$ = createExpression(REST, NULL, NULL); }
    | TIE { $$ = createExpression(TIE, NULL, NULL); }
    ;

note_no_list
    : note_no_list_param { $$ = createExpression(NOTE_NO_LIST, $1, NULL); }
    ;
note_no_list_param
    : note_no COMMA note_no { $$ = addRightExpression($1, $3); }
    | note_no_list_param COMMA note_no { $$ = addRightExpression($1, $3); }
    ;
note_no
    : NOTE_NO { $$ = createStringValue(NOTE_NO, $1); }
    ;

identifier
    : IDENTIFIER { $$ = createStringValue(IDENTIFIER, $1); }
    ;

%%
