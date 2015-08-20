%{
 
#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"

extern int yyerror(YYLTYPE *yylloc, void *scanner, ParserCallback callback, ParserErrorCallback errorCallback, const char *message);
 
#define CALLBACK(location, statement, ...) callback(yyget_extra(scanner), (ParseLocation *)location, statement, __VA_ARGS__)

%}

%output = "Parser.c"
%defines
 
%pure-parser
%lex-param   { void *scanner }
%parse-param { void *scanner }
%parse-param { ParserCallback callback }
%parse-param { ParserErrorCallback errorCallback }
%locations

%union {
    int i;
    float f;
    char *s;
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
%token VOLUME
%token PAN
%token CHORUS
%token REVERB
%token TRANSPOSE
%token KEY

%token PLUS
%token MINUS
%token DIVISION
%token SEMICOLON

%token <s>IDENTIFIER
%token <s>PATTERN_ID
%token <s>PHRASE_ID
%token <s>KEY_SIGN
%token <s>NOTE

%token EOL

%%
 
input
    :
    | statement_list
    ;

statement_list
    : statement
    | statement_list statement
    ;

statement
    : TITLE STRING                  { CALLBACK(&@$, StatementTitle, $2); }
    | RESOLUTION INTEGER            { CALLBACK(&@$, StatementResolution, $2); }
    | TEMPO FLOAT                   { CALLBACK(&@$, StatementTempo, $2); }
    | TEMPO INTEGER                 { CALLBACK(&@$, StatementTempo, (float)$2); }
    | TIME INTEGER DIVISION INTEGER { CALLBACK(&@$, StatementTimeSign, $2, $4); }
    | INTEGER                       { CALLBACK(&@$, StatementMeasure, $1); }
    | MARKER STRING                 { CALLBACK(&@$, StatementMarker, $2); }
    | PATTERN_ID                    { CALLBACK(&@$, StatementPattern, $1); }
    | DEFINE PATTERN_ID             { CALLBACK(&@$, StatementPatternDefine, $2); }
    | END                           { CALLBACK(&@$, StatementEnd, NULL); }
    | CHANNEL INTEGER               { CALLBACK(&@$, StatementChannel, $2); }
    | VOICE INTEGER INTEGER INTEGER { CALLBACK(&@$, StatementVoice, $2, $3, $4); }
    | VOLUME INTEGER                { CALLBACK(&@$, StatementVolume, $2); }
    | PAN INTEGER                   { CALLBACK(&@$, StatementPan, $2); }
    | CHORUS INTEGER                { CALLBACK(&@$, StatementChrous, $2); }
    | REVERB INTEGER                { CALLBACK(&@$, StatementReverb, $2); }
    | PHRASE_ID                     { CALLBACK(&@$, StatementPhrase, $1); }
    | DEFINE PHRASE_ID              { CALLBACK(&@$, StatementPhraseDefine, $2); }
    | TRANSPOSE INTEGER             { CALLBACK(&@$, StatementTranspose, $2); }
    | TRANSPOSE PLUS INTEGER        { CALLBACK(&@$, StatementTranspose, $3); }
    | TRANSPOSE MINUS INTEGER       { CALLBACK(&@$, StatementTranspose, -$3); }
    | KEY KEY_SIGN                  { CALLBACK(&@$, StatementKey, $2); }
    | NOTE                          { CALLBACK(&@$, StatementNote, $1, -1, -1, -1); }
    | NOTE MINUS                    { CALLBACK(&@$, StatementNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS            { CALLBACK(&@$, StatementNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   MINUS    { CALLBACK(&@$, StatementNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   INTEGER  { CALLBACK(&@$, StatementNote, $1, -1, -1, $4); }
    | NOTE MINUS   INTEGER          { CALLBACK(&@$, StatementNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER MINUS    { CALLBACK(&@$, StatementNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER INTEGER  { CALLBACK(&@$, StatementNote, $1, -1, $3, $4); }
    | NOTE INTEGER                  { CALLBACK(&@$, StatementNote, $1, $2, -1, -1); }
    | NOTE INTEGER INTEGER          { CALLBACK(&@$, StatementNote, $1, $2, $3, -1); }
    | NOTE INTEGER INTEGER INTEGER  { CALLBACK(&@$, StatementNote, $1, $2, $3, $4); }
    | NOTE INTEGER INTEGER MINUS    { CALLBACK(&@$, StatementNote, $1, $2, $3, -1); }
    | NOTE INTEGER MINUS            { CALLBACK(&@$, StatementNote, $1, $2, -1, -1); }
    | NOTE INTEGER MINUS   INTEGER  { CALLBACK(&@$, StatementNote, $1, $2, -1, $4); }
    | NOTE INTEGER MINUS   MINUS    { CALLBACK(&@$, StatementNote, $1, $2, -1, -1); }
    | MINUS INTEGER                 { CALLBACK(&@$, StatementRest, $2); }
    | EOL
    | SEMICOLON
    ;

%%

int yyerror(YYLTYPE *yylloc, void *scanner, ParserCallback callback, ParserErrorCallback errorCallback, const char *message)
{
    errorCallback(yyget_extra(scanner), (ParseLocation *)yylloc, message);
    return 0;
}
