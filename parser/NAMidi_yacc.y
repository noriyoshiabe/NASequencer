%{
 
#include "NAMidiParser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"
#include "NAMidiExpression.h"

#include <ctype.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, Expression **expression, const char *message);

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
                                                  $$ = NAMidiExprStatementList(filepath, &@$);
                                                  ExpressionAddChild($$, $1);
                                              }
                                          }
    | statement_list statement            {
                                              if (!$2) YYABORT;

                                              if (NAMidiExprIsStatementList($2)) {
                                                  NAMidiExprStatementListAppend($1, $2);
                                                  $$ = $1;
                                              }
                                              else {
                                                  $$ = ExpressionAddChild($1, $2);
                                              }
                                          }

    ;

statement
    : TITLE STRING                        { $$ = NAMidiExprTitle(filepath, &@$, $2); }
    | RESOLUTION INTEGER                  { $$ = NAMidiExprResolution(filepath, &@$, $2); }
    | TEMPO FLOAT                         { $$ = NAMidiExprTempo(filepath, &@$, $2); }
    | TEMPO INTEGER                       { $$ = NAMidiExprTempo(filepath, &@$, $2); }
    | TIME INTEGER DIVISION INTEGER       { $$ = NAMidiExprTimeSign(filepath, &@$, $2, $4); }
    | MARKER STRING                       { $$ = NAMidiExprMarker(filepath, &@$, $2); }
    | CHANNEL INTEGER                     { $$ = NAMidiExprChannel(filepath, &@$, $2); }
    | VOICE INTEGER INTEGER INTEGER       { $$ = NAMidiExprVoice(filepath, &@$, $2, $3, $4); }
    | SYNTH STRING                        { $$ = NAMidiExprSynth(filepath, &@$, $2); }
    | VOLUME INTEGER                      { $$ = NAMidiExprVolume(filepath, &@$, $2); }
    | PAN INTEGER                         { $$ = NAMidiExprPan(filepath, &@$, $2); }
    | PAN PLUS INTEGER                    { $$ = NAMidiExprPan(filepath, &@$, $3); }
    | PAN MINUS INTEGER                   { $$ = NAMidiExprPan(filepath, &@$, $3); }
    | CHORUS INTEGER                      { $$ = NAMidiExprChorus(filepath, &@$, $2); }
    | REVERB INTEGER                      { $$ = NAMidiExprReverb(filepath, &@$, $2); }
    | TRANSPOSE INTEGER                   { $$ = NAMidiExprTranspose(filepath, &@$, $2); }
    | TRANSPOSE PLUS INTEGER              { $$ = NAMidiExprTranspose(filepath, &@$, $3); }
    | TRANSPOSE MINUS INTEGER             { $$ = NAMidiExprTranspose(filepath, &@$, $3); }
    | KEY KEY_SIGN                        { $$ = NAMidiExprKeySign(filepath, &@$, $2); }
    | MINUS INTEGER                       { $$ = NAMidiExprRest(filepath, &@$, $2); }

    | note
    | pattern
    | pattern_expand
    | context

    | INCLUDE STRING                      {
                                              NAMidiParser *parser = NAMidi_get_extra(scanner);
                                              if (!NAMidiParserReadIncludeFile(parser, filepath, @$.first_line, @$.first_column, $2, &$$)) {
                                                  YYABORT;
                                              }
                                          }
    ;

note
    : NOTE                                { $$ = NAMidiExprNote(filepath, &@$, $1, -1, -1, -1); }
    | NOTE MINUS                          { $$ = NAMidiExprNote(filepath, &@$, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS                  { $$ = NAMidiExprNote(filepath, &@$, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   MINUS          { $$ = NAMidiExprNote(filepath, &@$, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   INTEGER        { $$ = NAMidiExprNote(filepath, &@$, $1, -1, -1, $4); }
    | NOTE MINUS   INTEGER                { $$ = NAMidiExprNote(filepath, &@$, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER MINUS          { $$ = NAMidiExprNote(filepath, &@$, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER INTEGER        { $$ = NAMidiExprNote(filepath, &@$, $1, -1, $3, $4); }
    | NOTE INTEGER                        { $$ = NAMidiExprNote(filepath, &@$, $1, $2, -1, -1); }
    | NOTE INTEGER INTEGER                { $$ = NAMidiExprNote(filepath, &@$, $1, $2, $3, -1); }
    | NOTE INTEGER INTEGER INTEGER        { $$ = NAMidiExprNote(filepath, &@$, $1, $2, $3, $4); }
    | NOTE INTEGER INTEGER MINUS          { $$ = NAMidiExprNote(filepath, &@$, $1, $2, $3, -1); }
    | NOTE INTEGER MINUS                  { $$ = NAMidiExprNote(filepath, &@$, $1, $2, -1, -1); }
    | NOTE INTEGER MINUS   INTEGER        { $$ = NAMidiExprNote(filepath, &@$, $1, $2, -1, $4); }
    | NOTE INTEGER MINUS   MINUS          { $$ = NAMidiExprNote(filepath, &@$, $1, $2, -1, -1); }
    ;

pattern
    : DEFINE IDENTIFIER statement_list END
                                          { $$ = NAMidiExprPattern(filepath, &@$, $2, $3); }
    ;

pattern_expand
    : IDENTIFIER                          { $$ = NAMidiExprPatternExpand(filepath, &@$, $1, NULL); }
    | IDENTIFIER LPAREN context_id_list RPAREN
                                          { $$ = NAMidiExprPatternExpand(filepath, &@$, $1, $3); }
    ;

context
    : context_id_list LCURLY statement_list RCURLY
                                          { $$ = NAMidiExprContext(filepath, &@$, $1, $3); }
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
    : IDENTIFIER                          { $$ = strdup($1); }
    | DEFAULT                             { $$ = strdup("default"); }
    ;

%%
