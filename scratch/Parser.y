%{
 
#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"

extern int yyerror(YYLTYPE *yylloc, void *scanner, ParserCallback callback, ParserErrorCallback errorCallback, const char *message);
 
#define CALLBACK(location, type, ...) \
    if (!callback(yyget_extra(scanner), (ParseLocation *)location, type, __VA_ARGS__)) { \
        return 1;\
    }

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
    : TITLE STRING                  { CALLBACK(&@$, StatementTypeTitle, $2); }
    | RESOLUTION INTEGER            { CALLBACK(&@$, StatementTypeResolution, $2); }
    | TEMPO FLOAT                   { CALLBACK(&@$, StatementTypeTempo, $2); }
    | TEMPO INTEGER                 { CALLBACK(&@$, StatementTypeTempo, (float)$2); }
    | TIME INTEGER DIVISION INTEGER { CALLBACK(&@$, StatementTypeTimeSign, $2, $4); }
    | INTEGER                       { CALLBACK(&@$, StatementTypeMeasure, $1); }
    | MARKER STRING                 { CALLBACK(&@$, StatementTypeMarker, $2); }
    | PATTERN_ID                    { CALLBACK(&@$, StatementTypePattern, $1); }
    | DEFINE PATTERN_ID             { CALLBACK(&@$, StatementTypePatternDefine, $2); }
    | END                           { CALLBACK(&@$, StatementTypeEnd, NULL); }
    | CHANNEL INTEGER               { CALLBACK(&@$, StatementTypeChannel, $2); }
    | VOICE INTEGER INTEGER INTEGER { CALLBACK(&@$, StatementTypeVoice, $2, $3, $4); }
    | VOLUME INTEGER                { CALLBACK(&@$, StatementTypeVolume, $2); }
    | PAN INTEGER                   { CALLBACK(&@$, StatementTypePan, $2); }
    | CHORUS INTEGER                { CALLBACK(&@$, StatementTypeChrous, $2); }
    | REVERB INTEGER                { CALLBACK(&@$, StatementTypeReverb, $2); }
    | PHRASE_ID                     { CALLBACK(&@$, StatementTypePhrase, $1); }
    | DEFINE PHRASE_ID              { CALLBACK(&@$, StatementTypePhraseDefine, $2); }
    | TRANSPOSE INTEGER             { CALLBACK(&@$, StatementTypeTranspose, $2); }
    | TRANSPOSE PLUS INTEGER        { CALLBACK(&@$, StatementTypeTranspose, $3); }
    | TRANSPOSE MINUS INTEGER       { CALLBACK(&@$, StatementTypeTranspose, -$3); }
    | KEY KEY_SIGN                  { CALLBACK(&@$, StatementTypeKey, $2); }
    | NOTE                          { CALLBACK(&@$, StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS                    { CALLBACK(&@$, StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS            { CALLBACK(&@$, StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   MINUS    { CALLBACK(&@$, StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   INTEGER  { CALLBACK(&@$, StatementTypeNote, $1, -1, -1, $4); }
    | NOTE MINUS   INTEGER          { CALLBACK(&@$, StatementTypeNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER MINUS    { CALLBACK(&@$, StatementTypeNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER INTEGER  { CALLBACK(&@$, StatementTypeNote, $1, -1, $3, $4); }
    | NOTE INTEGER                  { CALLBACK(&@$, StatementTypeNote, $1, $2, -1, -1); }
    | NOTE INTEGER INTEGER          { CALLBACK(&@$, StatementTypeNote, $1, $2, $3, -1); }
    | NOTE INTEGER INTEGER INTEGER  { CALLBACK(&@$, StatementTypeNote, $1, $2, $3, $4); }
    | NOTE INTEGER INTEGER MINUS    { CALLBACK(&@$, StatementTypeNote, $1, $2, $3, -1); }
    | NOTE INTEGER MINUS            { CALLBACK(&@$, StatementTypeNote, $1, $2, -1, -1); }
    | NOTE INTEGER MINUS   INTEGER  { CALLBACK(&@$, StatementTypeNote, $1, $2, -1, $4); }
    | NOTE INTEGER MINUS   MINUS    { CALLBACK(&@$, StatementTypeNote, $1, $2, -1, -1); }
    | MINUS INTEGER                 { CALLBACK(&@$, StatementTypeRest, $2); }
    | EOL
    | SEMICOLON
    ;

%%

int yyerror(YYLTYPE *yylloc, void *scanner, ParserCallback callback, ParserErrorCallback errorCallback, const char *message)
{
    errorCallback(yyget_extra(scanner), (ParseLocation *)yylloc, message);
    return 0;
}
