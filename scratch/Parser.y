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

%type <expression> statement_list
%type <expression> statement
%type <expression> integer
%type <expression> float
%type <expression> note_block
%type <expression> note_list
%type <expression> note
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
    | statement_list statement { $$ = ExpressionAddSibling($1, $2); }
    ;

statement
    : note_block
    | REST              { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeRest, NULL); }
    | quantize          { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeQuantize, $1); }
    | OCTAVE_SHIFT      { $$ = ExpressionCreateStringValue(scanner, &@$, ExpressionTypeOctaveShift, $1); }
    | KEY KEY_SIGN      { $$ = ExpressionCreateStringValue(scanner, &@$, ExpressionTypeKey, $2); }
    | TIME time_sign    { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTime, $2); }
    | TEMPO integer     { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTempo, $2); }
    | TEMPO float       { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTempo, $2); }
    | MARKER STRING     { $$ = ExpressionCreateTrimmedStringValue(scanner, &@$, ExpressionTypeMarker, $2); }
    | CHANNEL INTEGER   { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeChannel, $2); }
    | VELOCITY INTEGER  { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeVelocity, $2); }
    | GATETIME INTEGER  { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeGatetime, $2); }
    | GATETIME AUTO     { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeGatetimeAuto, NULL); }
    | OCTAVE INTEGER    { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeOctave, $2); }
    | OCTAVE PLUS INTEGER  { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeOctave, $3); }
    | OCTAVE MINUS INTEGER { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeOctave, -$3); }
    | LOCATION location_value    { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeLocation, $2); }
    | statement MULTIPLY INTEGER { $$ = ExpressionAddChild(ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeRepeat, $3), $1); }
    | block
    | note_block COMMA block { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeParallel, ExpressionAddSibling($1, $3)); }
    | block COMMA note_block { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeParallel, ExpressionAddSibling($1, $3)); }
    | block COMMA block { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeParallel, ExpressionAddSibling($1, $3)); }
    | identifier ASSIGN block { $$ = ExpressionCreate(scanner, &@$, ExpressionTypePatternDefine, ExpressionAddSibling($1, $3)); }
    | identifier                            { $$ = ExpressionCreate(scanner, &@$, ExpressionTypePatternExpand, $1); }
    | identifier pattern_expand_param_list  { $$ = ExpressionCreate(scanner, &@$, ExpressionTypePatternExpand, ExpressionAddSibling($1, $2)); }
    ;

integer
    : INTEGER { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeInteger, $1); }
    ;

float
    : FLOAT { $$ = ExpressionCreateFloatValue(scanner, &@$, ExpressionTypeFloat, $1); }
    ;

note_block
    : note_list { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeNoteBlock, $1); }
    ;

note_list
    : note { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeNoteList, $1); }
    | note_list TIE { $$ = ExpressionAddSibling($1, ExpressionCreate(scanner, &@$, ExpressionTypeTie, NULL)); }
    ;

note
    : NOTE            { $$ = ExpressionCreateStringValue(scanner, &@$, ExpressionTypeNote, $1); }
    | note COMMA NOTE { $$ = ExpressionAddSibling($1, ExpressionCreateStringValue(scanner, &@$, ExpressionTypeNote, $3)); }
    ;

time_sign
    : integer DIVISION integer { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTimeSign, ExpressionAddSibling($1, $3)); }
    ;

quantize_time_sign
    : integer DIVISION integer { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTimeSign, ExpressionAddSibling($1, $3)); }
    | DIVISION integer { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeTimeSign, ExpressionAddSibling(ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeInteger, 1), $2)); }
    ;

quantize
    : quantize_time_sign
    | quantize_time_sign DOT { $$ = ExpressionAddSibling($1, ExpressionCreate(scanner, &@$, ExpressionTypeDot, NULL)) }
    | quantize_time_sign integer { $$ = ExpressionAddSibling($1, $2) }
    ;

location_value
    : INTEGER                             { $$ = ExpressionCreateIntegerValue(scanner, &@$, ExpressionTypeMeasure, $1); }
    | time_sign
    | location_value PLUS location_value  { $$ = ExpressionCreate(scanner, &@$, ExpressionTypePlus, ExpressionAddSibling($1, $3)); }
    | location_value MINUS location_value { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeMinus, ExpressionAddSibling($1, $3)); }
    ;

identifier
    : IDENTIFIER { $$ = ExpressionCreateStringValue(scanner, &@$, ExpressionTypeString, $1); }
    ;

block
    : LCURLY statement_list RCURLY { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeBlock, $2); }
    ;

pattern_expand_param_list
    : pattern_expand_param
    | pattern_expand_param_list pattern_expand_param { $$ = ExpressionAddSibling($1, $2) }
    ;

pattern_expand_param
    : OFFSET length_value { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeOffset, $2); }
    | LENGTH length_value { $$ = ExpressionCreate(scanner, &@$, ExpressionTypeLength, $2); }
    ;

length_value
    : location_value
    ;

%%
