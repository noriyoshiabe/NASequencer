%{
 
#include "NAMidiParser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"
#include "NAMidiExpression.h"
#include "NAUtil.h"

#include <ctype.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, Expression **expression, const char *message);

#define PERSER() ((NAMidiParser *)NAMidi_get_extra(scanner))
#define LOC(yylloc) (&(ParseLocation){filepath, yylloc.first_line, yylloc.first_column})

%}

%name-prefix = "NAMidi_"
%output = "NAMidi_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { const char *filepath }
%parse-param { Expression **expression }
%locations

%union {
    int i;
    float f;
    char *s;
    Expression *expression;
    NAArray *array;
}

%token <i>INTEGER
%token <f>FLOAT
%token <s>STRING

%token RESOLUTION
%token TITLE
%token TEMPO
%token TIME
%token MARKER
%token DEFINE
%token END
%token CHANNEL
%token VOICE
%token SYNTH
%token VOLUME
%token PAN
%token CHORUS
%token REVERB
%token TRANSPOSE
%token KEY
%token DEFAULT

%token PLUS
%token MINUS
%token DIVISION
%token SEMICOLON
%token LPAREN
%token RPAREN
%token LCURLY
%token RCURLY
%token COMMA

%token INCLUDE

%token <s>IDENTIFIER
%token <s>KEY_SIGN
%token <s>NOTE

%token EOL

%type <expression> statement_list statement
%type <expression> note
%type <expression> pattern pattern_expand
%type <expression> context
%type <array>      context_id_list
%type <s>          context_id
%type <s>          identifier

%%
 
input
    :
    | statement_list                      { *expression = $1; }
    ;

statement_list
    : statement                           {
                                              if (!$1) YYABORT;

                                              if (NAMidiExprIsStatementList($1)) {
                                                  $$ = $1;
                                              }
                                              else {
                                                  $$ = NAMidiExprStatementList(PERSER(), LOC(@$));
                                                  ExpressionAddChild($$, $1);
                                              }
                                          }
    | statement_list statement            {
                                              if (!$2) YYABORT;

                                              if (NAMidiExprIsStatementList($2)) {
                                                  $$ = NAMidiExprStatementListMarge($1, $2);
                                              }
                                              else {
                                                  $$ = ExpressionAddChild($1, $2);
                                              }
                                          }

    ;

statement
    : TITLE STRING                        { $$ = NAMidiExprTitle(PERSER(), LOC(@$), $2); }
    | RESOLUTION INTEGER                  { $$ = NAMidiExprResolution(PERSER(), LOC(@$), $2); }
    | TEMPO FLOAT                         { $$ = NAMidiExprTempo(PERSER(), LOC(@$), $2); }
    | TEMPO INTEGER                       { $$ = NAMidiExprTempo(PERSER(), LOC(@$), $2); }
    | TIME INTEGER DIVISION INTEGER       { $$ = NAMidiExprTimeSign(PERSER(), LOC(@$), $2, $4); }
    | MARKER STRING                       { $$ = NAMidiExprMarker(PERSER(), LOC(@$), $2); }
    | CHANNEL INTEGER                     { $$ = NAMidiExprChannel(PERSER(), LOC(@$), $2); }
    | VOICE INTEGER INTEGER INTEGER       { $$ = NAMidiExprVoice(PERSER(), LOC(@$), $2, $3, $4); }
    | SYNTH STRING                        { $$ = NAMidiExprSynth(PERSER(), LOC(@$), $2); }
    | VOLUME INTEGER                      { $$ = NAMidiExprVolume(PERSER(), LOC(@$), $2); }
    | PAN INTEGER                         { $$ = NAMidiExprPan(PERSER(), LOC(@$), $2); }
    | PAN PLUS INTEGER                    { $$ = NAMidiExprPan(PERSER(), LOC(@$), $3); }
    | PAN MINUS INTEGER                   { $$ = NAMidiExprPan(PERSER(), LOC(@$), $3); }
    | CHORUS INTEGER                      { $$ = NAMidiExprChorus(PERSER(), LOC(@$), $2); }
    | REVERB INTEGER                      { $$ = NAMidiExprReverb(PERSER(), LOC(@$), $2); }
    | TRANSPOSE INTEGER                   { $$ = NAMidiExprTranspose(PERSER(), LOC(@$), $2); }
    | TRANSPOSE PLUS INTEGER              { $$ = NAMidiExprTranspose(PERSER(), LOC(@$), $3); }
    | TRANSPOSE MINUS INTEGER             { $$ = NAMidiExprTranspose(PERSER(), LOC(@$), $3); }
    | KEY KEY_SIGN                        { $$ = NAMidiExprKeySign(PERSER(), LOC(@$), $2); }
    | MINUS INTEGER                       { $$ = NAMidiExprRest(PERSER(), LOC(@$), $2); }

    | note
    | pattern
    | pattern_expand
    | context

    | INCLUDE STRING                      {
                                              if (!NAMidiParserReadIncludeFile(PERSER(), LOC(@$), $2, &$$)) {
                                                  YYABORT;
                                              }
                                          }
    ;

note
    : NOTE                                { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, -1, -1, -1); }
    | NOTE MINUS                          { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, -1, -1, -1); }
    | NOTE MINUS   MINUS                  { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   MINUS          { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   INTEGER        { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, -1, -1, $4); }
    | NOTE MINUS   INTEGER                { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER MINUS          { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER INTEGER        { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, -1, $3, $4); }
    | NOTE INTEGER                        { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, $2, -1, -1); }
    | NOTE INTEGER INTEGER                { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, $2, $3, -1); }
    | NOTE INTEGER INTEGER INTEGER        { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, $2, $3, $4); }
    | NOTE INTEGER INTEGER MINUS          { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, $2, $3, -1); }
    | NOTE INTEGER MINUS                  { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, $2, -1, -1); }
    | NOTE INTEGER MINUS   INTEGER        { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, $2, -1, $4); }
    | NOTE INTEGER MINUS   MINUS          { $$ = NAMidiExprNote(PERSER(), LOC(@$), $1, $2, -1, -1); }
    ;

pattern
    : DEFINE identifier statement_list END
                                          { $$ = NAMidiExprPattern(PERSER(), LOC(@$), $2, $3); }
    ;

pattern_expand
    : identifier                          { $$ = NAMidiExprPatternExpand(PERSER(), LOC(@$), $1, NULL); }
    | identifier LPAREN context_id_list RPAREN
                                          { $$ = NAMidiExprPatternExpand(PERSER(), LOC(@$), $1, $3); }
    ;

context
    : context_id_list LCURLY statement_list RCURLY
                                          { $$ = NAMidiExprContext(PERSER(), LOC(@$), $1, $3); }
    ;

context_id_list
    : context_id                          {
                                              $$ = NAArrayCreate(4, NADescriptionCString);
                                              NAArrayAppend($$, $1);
                                          }
    | context_id_list COMMA context_id    {
                                              $$ = $1;
                                              NAArrayAppend($$, $3);
                                          }
    ;

context_id
    : identifier                          { $$ = $1; }
    | DEFAULT                             { $$ = strdup("default"); }
    ;

identifier
    : IDENTIFIER                          { $$ = NAUtilToLowerCase($1); }
    ;

%%
