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
%token <f>FLOAT
%token <s>STRING

%token <s>NOTE
%token <s>KEY_SIGN
%token <s>OCTAVE_SHIFT

%token KEY
%token TIME
%token TEMPO
%token MARKER
%token CHANNEL
%token VELOCITY
%token GATETIME
%token AUTO
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
%token DOT
%token LCURLY
%token RCURLY

%token <s>IDENTIFIER

%token EOL

%token TOKEN_END

%type <expression> statement_list
%type <expression> statement
%type <expression> integer
%type <expression> float
%type <expression> time_sign
%type <expression> quantize_time_sign
%type <expression> quantize
%type <expression> location_value
%type <expression> identifier
%type <expression> block
%type <expression> pattern_expand_param
%type <expression> pattern_expand_param_list
%type <expression> length_value

%left PLUS
%left MINUS
%left MULTIPLY
%left DIVISION

%%
 
input
    :
    | statement_list { *expression = $1; }
    ;

statement_list
    : statement
    | statement_list statement { $$ = ExpressionAddRight($1, $2); }
    ;

statement
    : NOTE              { $$ = ExpressionCreateStringValue(scanner, &@$, ExpressionTypeNote, $1); }
    | quantize          { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeQuantize, $1, NULL); }
    | OCTAVE_SHIFT      { $$ = ExpressionCreateStringValue(scanner, &@$, ExpressionTypeOctaveShift, $1); }
    | KEY KEY_SIGN      { $$ = ExpressionCreateStringValue(scanner, &@$, ExpressionTypeKey, $2); }
    | TIME time_sign    { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTime, $2, NULL); }
    | TEMPO integer     { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTempo, $2, NULL); }
    | TEMPO float       { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTempo, $2, NULL); }
    | MARKER STRING     { $$ = ExpressionCreateTrimmedStringValue(scanner, &@$, ExpressionTypeMarker, $2); }
    | CHANNEL INTEGER   { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeChannel, $2); }
    | VELOCITY INTEGER  { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeVelocity, $2); }
    | GATETIME INTEGER  { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeGatetime, $2); }
    | GATETIME AUTO     { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeGatetimeAuto, NULL, NULL); }
    | OCTAVE INTEGER    { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeOctave, $2); }
    | OCTAVE PLUS INTEGER  { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeOctave, $3); }
    | OCTAVE MINUS INTEGER { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeOctave, -$3); }
    | REST              { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeRest, NULL, NULL); }
    | TIE               { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTie, NULL, NULL); }
    | LOCATION location_value    { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeLocation, $2, NULL); }
    | statement MULTIPLY INTEGER { $$ = ExpressionAddLeft($1, ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeRepeat, $3)); }
    | statement COMMA statement  { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeParallel, ExpressionAddRight($1, $3), NULL); }
    | block
    | identifier ASSIGN block { $$ = ExpressionCreate(scanner, &@$, ExpressionTypePatternDefine, ExpressionAddRight($1, $3), NULL); }
    | identifier                            { $$ = ExpressionCreate(scanner, &@$, ExpressionTypePatternExpand, $1, NULL); }
    | identifier pattern_expand_param_list  { $$ = ExpressionCreate(scanner, &@$, ExpressionTypePatternExpand, ExpressionAddRight($1, $2), NULL); }
    ;

integer
    : INTEGER { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeInteger, $1); }
    ;

float
    : FLOAT { $$ = ExpressionCreateFloatValue(scanner, &@$, ExpressionTypeFloat, $1); }
    ;

time_sign
    : integer DIVISION integer { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTimeSign, ExpressionAddRight($1, $3), NULL); }
    ;

quantize_time_sign
    : integer DIVISION integer { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTimeSign, ExpressionAddRight($1, $3), NULL); }
    | DIVISION integer { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTimeSign, ExpressionAddRight(ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeInteger, 1), $2), NULL); }
    ;

quantize
    : quantize_time_sign
    | quantize_time_sign DOT { $$ = ExpressionAddRight($1, ExpressionCreate(scanner, &@$, ExpressionTypeDot, NULL, NULL)) }
    | quantize_time_sign integer { $$ = ExpressionAddRight($1, $2) }
    ;

location_value
    : integer
    | time_sign
    | location_value PLUS location_value  { $$ = ExpressionCreate(scanner, &@$, ExpressionTypePlus, ExpressionAddRight($1, $3), NULL); }
    | location_value MINUS location_value { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeMinus, ExpressionAddRight($1, $3), NULL); }
    ;

identifier
    : IDENTIFIER { $$ = ExpressionCreateStringValue(scanner, &@$, ExpressionTypeString, $1); }
    ;

block
    : LCURLY statement_list RCURLY { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeBlock, $2, NULL); }
    ;

pattern_expand_param_list
    : pattern_expand_param
    | pattern_expand_param_list pattern_expand_param { $$ = ExpressionAddRight($1, $2) }
    ;

pattern_expand_param
    : OFFSET length_value { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeOffset, $2, NULL); }
    | LENGTH length_value { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeLength, $2, NULL); }
    ;

length_value
    : location_value
    ;

%%
