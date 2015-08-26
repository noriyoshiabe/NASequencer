%{
 
#include "NAMidiParser.h"
#include "NAMidiLexer.h"
#include "Parser.h"

extern int NAMidi_error(yyscan_t scanner, const char *message);
extern int NAMidi_get_column(yyscan_t scanner);

static bool callbackStatement(yyscan_t scanner, StatementType type, ...);
#define CALLBACK(type, ...) \
    if (!callbackStatement(scanner, type, __VA_ARGS__)) { \
        return 1;\
    }

%}

%name-prefix = "NAMidi_"
%output = "NAMidiParser.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }

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
%token TRACK
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
%token COLON
%token SEMICOLON

%token INCLUDE

%token <s>IDENTIFIER
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
    : TITLE STRING                  { CALLBACK(StatementTypeTitle, $2); }
    | RESOLUTION INTEGER            { CALLBACK(StatementTypeResolution, $2); }
    | TEMPO FLOAT                   { CALLBACK(StatementTypeTempo, $2); }
    | TEMPO INTEGER                 { CALLBACK(StatementTypeTempo, (float)$2); }
    | TIME INTEGER DIVISION INTEGER { CALLBACK(StatementTypeTimeSign, $2, $4); }
    | INTEGER COLON                 { CALLBACK(StatementTypeMeasure, $1); }
    | MARKER STRING                 { CALLBACK(StatementTypeMarker, $2); }
    | IDENTIFIER                    { CALLBACK(StatementTypePattern, $1); }
    | DEFINE IDENTIFIER             { CALLBACK(StatementTypePatternDefine, $2); }
    | END                           { CALLBACK(StatementTypeEnd, NULL); }
    | TRACK INTEGER                 { CALLBACK(StatementTypeTrack, $2); }
    | CHANNEL INTEGER               { CALLBACK(StatementTypeChannel, $2); }
    | VOICE INTEGER INTEGER INTEGER { CALLBACK(StatementTypeVoice, $2, $3, $4); }
    | VOLUME INTEGER                { CALLBACK(StatementTypeVolume, $2); }
    | PAN INTEGER                   { CALLBACK(StatementTypePan, $2); }
    | PAN PLUS INTEGER              { CALLBACK(StatementTypePan, $3); }
    | PAN MINUS INTEGER             { CALLBACK(StatementTypePan, -$3); }
    | CHORUS INTEGER                { CALLBACK(StatementTypeChorus, $2); }
    | REVERB INTEGER                { CALLBACK(StatementTypeReverb, $2); }
    | TRANSPOSE INTEGER             { CALLBACK(StatementTypeTranspose, $2); }
    | TRANSPOSE PLUS INTEGER        { CALLBACK(StatementTypeTranspose, $3); }
    | TRANSPOSE MINUS INTEGER       { CALLBACK(StatementTypeTranspose, -$3); }
    | KEY KEY_SIGN                  { CALLBACK(StatementTypeKey, $2); }
    | NOTE                          { CALLBACK(StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS                    { CALLBACK(StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS            { CALLBACK(StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   MINUS    { CALLBACK(StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   INTEGER  { CALLBACK(StatementTypeNote, $1, -1, -1, $4); }
    | NOTE MINUS   INTEGER          { CALLBACK(StatementTypeNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER MINUS    { CALLBACK(StatementTypeNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER INTEGER  { CALLBACK(StatementTypeNote, $1, -1, $3, $4); }
    | NOTE INTEGER                  { CALLBACK(StatementTypeNote, $1, $2, -1, -1); }
    | NOTE INTEGER INTEGER          { CALLBACK(StatementTypeNote, $1, $2, $3, -1); }
    | NOTE INTEGER INTEGER INTEGER  { CALLBACK(StatementTypeNote, $1, $2, $3, $4); }
    | NOTE INTEGER INTEGER MINUS    { CALLBACK(StatementTypeNote, $1, $2, $3, -1); }
    | NOTE INTEGER MINUS            { CALLBACK(StatementTypeNote, $1, $2, -1, -1); }
    | NOTE INTEGER MINUS   INTEGER  { CALLBACK(StatementTypeNote, $1, $2, -1, $4); }
    | NOTE INTEGER MINUS   MINUS    { CALLBACK(StatementTypeNote, $1, $2, -1, -1); }
    | MINUS INTEGER                 { CALLBACK(StatementTypeRest, $2); }
    | EOL
    | SEMICOLON
    | INCLUDE STRING                { CALLBACK(StatementTypeInclude, $2); }
    ;

%%

int NAMidi_error(yyscan_t scanner, const char *message)
{
    ParseContext *context = NAMidi_get_extra(scanner);
    context->location = (ParseLocation){NAMidi_get_lineno(scanner), NAMidi_get_column(scanner)};
    context->handler->error(context->receiver, context, ParseErrorSyntaxError);
    return 0;
}

static bool callbackStatement(yyscan_t scanner, StatementType type, ...)
{
    ParseContext *context = NAMidi_get_extra(scanner);
    context->location = (ParseLocation){NAMidi_get_lineno(scanner), NAMidi_get_column(scanner)};

    va_list argList;
    va_start(argList, type);
    bool ret = context->handler->process(context->receiver, context, type, argList);
    va_end(argList);
    return ret;
}
