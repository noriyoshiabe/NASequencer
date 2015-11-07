%{
 
#include "NAMidiParser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

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
%type <expression> context context_id_list context_id

%%
 
input
    :
    | statement_list  { *expression = $1; }
    ;

statement_list
    : statement
    | statement_list statement { $$ = ExpressionAddSibling($1, $2); }
    ;

statement
    : TITLE STRING                        { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "title"); }
    | RESOLUTION INTEGER                  { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "reslution"); }
    | TEMPO FLOAT                         { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "tempo f"); }
    | TEMPO INTEGER                       { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "tempo i"); }
    | TIME INTEGER DIVISION INTEGER       { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "time"); }
    | MARKER STRING                       { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "marker"); }
    | CHANNEL INTEGER                     { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "channel"); }
    | VOICE INTEGER INTEGER INTEGER       { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "voice"); }
    | SYNTH STRING                        { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "synth"); }
    | VOLUME INTEGER                      { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "volume"); }
    | PAN INTEGER                         { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "pan"); }
    | PAN PLUS INTEGER                    { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "pan +"); }
    | PAN MINUS INTEGER                   { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "pan -"); }
    | CHORUS INTEGER                      { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "chorus"); }
    | REVERB INTEGER                      { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "reverb"); }
    | TRANSPOSE INTEGER                   { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "transpose"); }
    | TRANSPOSE PLUS INTEGER              { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "transpose +"); }
    | TRANSPOSE MINUS INTEGER             { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "transpose -"); }
    | KEY KEY_SIGN                        { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "key"); }
    | MINUS INTEGER                       { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "rest"); }

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
    : NOTE                                { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE MINUS                          { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE MINUS   MINUS                  { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE MINUS   MINUS   MINUS          { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE MINUS   MINUS   INTEGER        { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE MINUS   INTEGER                { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE MINUS   INTEGER MINUS          { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE MINUS   INTEGER INTEGER        { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE INTEGER                        { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE INTEGER INTEGER                { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE INTEGER INTEGER INTEGER        { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE INTEGER INTEGER MINUS          { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE INTEGER MINUS                  { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE INTEGER MINUS   INTEGER        { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    | NOTE INTEGER MINUS   MINUS          { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "note"); }
    ;

pattern
    : DEFINE IDENTIFIER statement_list END
                                          {
                                               Expression *expr = ExpressionCreate(filepath, &@$, sizeof(Expression), "def pettern");
                                               ExpressionAddChild(expr, $3);
                                               $$ = expr;
                                          }
    ;

pattern_expand
    : IDENTIFIER                          { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "pattern"); }
    | IDENTIFIER LPAREN context_id_list RPAREN
                                          { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "pattern + id_list"); }
    ;

context
    : context_id_list LCURLY statement_list RCURLY
                                          {
                                               Expression *expr = ExpressionCreate(filepath, &@$, sizeof(Expression), "context + id_list");
                                               ExpressionAddChild(expr, $3);
                                               $$ = expr;
                                          }
    ;

context_id_list
    : context_id
    | context_id_list COMMA context_id    { $$ = ExpressionAddSibling($1, $3); }
    ;

context_id
    : IDENTIFIER                          { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "ctx id"); }
    | DEFAULT                             { $$ = ExpressionCreate(filepath, &@$, sizeof(Expression), "default"); }
    ;

%%
