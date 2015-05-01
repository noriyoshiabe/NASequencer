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
    char *s;
    Expression *expression;
}

%token TOKEN_BEGIN

%token <i>INTEGER
%token <s>DECIMAL
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
    : statement_list { *expression = $1; }
    ;

statement_list
    : statement
    | statement_list statement { $$ = ExpressionAddRight($1, $2); }
    ;

statement
    : NOTE              { $$ = ExpressionCreateStringValue(&@$, ExpressionTypeNote, $1); }
    | quantize          { $$ = ExpressionCreate(&@$, ExpressionTypeQuantize, $1, NULL); }
    | OCTAVE_SHIFT      { $$ = ExpressionCreateStringValue(&@$, ExpressionTypeOctaveShift, $1); }
    | KEY KEY_SIGN      { $$ = ExpressionCreateStringValue(&@$, ExpressionTypeKey, $2); }
    | TIME time_sign    { $$ = ExpressionCreate(&@$, ExpressionTypeTime, $2, NULL); }
    | TEMPO integer     { $$ = ExpressionCreate(&@$, ExpressionTypeTempo, $2, NULL); }
    | TEMPO float       { $$ = ExpressionCreate(&@$, ExpressionTypeTempo, $2, NULL); }
    | MARKER STRING     { $$ = ExpressionCreateTrimmedStringValue(&@$, ExpressionTypeMarker, $2); }
    | CHANNEL INTEGER   { $$ = ExpressionCreateIntegerValue(&@$, ExpressionTypeChannel, $2); }
    | VELOCITY INTEGER  { $$ = ExpressionCreateIntegerValue(&@$, ExpressionTypeVelocity, $2); }
    | GATETIME INTEGER  { $$ = ExpressionCreateIntegerValue(&@$, ExpressionTypeGatetime, $2); }
    | GATETIME AUTO     { $$ = ExpressionCreate(&@$, ExpressionTypeGatetimeAuto, NULL, NULL); }
    | OCTAVE INTEGER    { $$ = ExpressionCreateIntegerValue(&@$, ExpressionTypeOctave, $2); }
    | OCTAVE PLUS INTEGER  { $$ = ExpressionCreateIntegerValue(&@$, ExpressionTypeOctave, $3); }
    | OCTAVE MINUS INTEGER { $$ = ExpressionCreateIntegerValue(&@$, ExpressionTypeOctave, -$3); }
    | REST              { $$ = ExpressionCreate(&@$, ExpressionTypeRest, NULL, NULL); }
    | TIE               { $$ = ExpressionCreate(&@$, ExpressionTypeTie, NULL, NULL); }
    | LOCATION location_value    { $$ = ExpressionCreate(&@$, ExpressionTypeLocation, $2, NULL); }
    | statement MULTIPLY INTEGER { $$ = ExpressionAddLeft($1, ExpressionCreateIntegerValue(&@$, ExpressionTypeRepeat, $3)); }
    | statement COMMA statement  { $$ = ExpressionCreate(&@$, ExpressionTypeParallel, ExpressionAddRight($1, $3), NULL); }
    | block
    | identifier ASSIGN block { $$ = ExpressionCreate(&@$, ExpressionTypePatternDefine, ExpressionAddRight($1, $3), NULL); }
    | identifier                            { $$ = ExpressionCreate(&@$, ExpressionTypePatternExpand, $1, NULL); }
    | identifier pattern_expand_param_list  { $$ = ExpressionCreate(&@$, ExpressionTypePatternExpand, ExpressionAddRight($1, $2), NULL); }
    ;

integer
    : INTEGER { $$ = ExpressionCreateIntegerValue(&@$, ExpressionTypeInteger, $1); }
    ;

float
    : INTEGER DOT DECIMAL { $$ = ExpressionCreateFloatValue(&@$, ExpressionTypeFloat, $1, $3); }
    ;

time_sign
    : integer DIVISION integer { $$ = ExpressionCreate(&@$, ExpressionTypeTimeSign, ExpressionAddRight($1, $3), NULL); }
    ;

quantize_time_sign
    : integer DIVISION integer { $$ = ExpressionCreate(&@$, ExpressionTypeTimeSign, ExpressionAddRight($1, $3), NULL); }
    | DIVISION integer { $$ = ExpressionCreate(&@$, ExpressionTypeTimeSign, ExpressionAddRight(ExpressionCreateIntegerValue(&@$, ExpressionTypeInteger, 1), $2), NULL); }
    ;

quantize
    : quantize_time_sign
    | quantize_time_sign DOT { $$ = ExpressionAddRight($1, ExpressionCreate(&@$, ExpressionTypeDot, NULL, NULL)) }
    ;

location_value
    : integer
    | time_sign
    | location_value PLUS location_value  { $$ = ExpressionCreate(&@$, ExpressionTypePlus, ExpressionAddRight($1, $3), NULL); }
    | location_value MINUS location_value { $$ = ExpressionCreate(&@$, ExpressionTypeMinus, ExpressionAddRight($1, $3), NULL); }
    ;

identifier
    : IDENTIFIER { $$ = ExpressionCreateStringValue(&@$, ExpressionTypeString, $1); }
    ;

block
    : LCURLY statement_list RCURLY { $$ = ExpressionCreate(&@$, ExpressionTypeBlock, $2, NULL); }
    ;

pattern_expand_param_list
    : pattern_expand_param
    | pattern_expand_param_list pattern_expand_param { $$ = ExpressionAddRight($1, $2) }
    ;

pattern_expand_param
    : OFFSET length_value { $$ = ExpressionCreate(&@$, ExpressionTypeOffset, $2, NULL); }
    | LENGTH length_value { $$ = ExpressionCreate(&@$, ExpressionTypeLength, $2, NULL); }
    ;

length_value
    : location_value
    ;

%%
