%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"
#include "NAMidiParser.h"

#include <ctype.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *message);

static bool callProcess(YYLTYPE *yylloc, yyscan_t scanner, StatementType type, ...);
static void postProcess(YYLTYPE *yylloc, yyscan_t scanner, StatementType type, ...);

#define Process(yylloc, type, ...) \
    bool success = callProcess(yylloc, scanner, type, __VA_ARGS__); \
    postProcess(yylloc, scanner, type, __VA_ARGS__); \
    if (!success) { \
        return 1;\
    }

%}

%name-prefix = "NAMidi_"
%output = "NAMidi_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
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
%token TRACK
%token CHANNEL
%token VOICE
%token SYNTH
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
    : TITLE STRING                  { Process(&@$, StatementTypeTitle, $2); }
    | RESOLUTION INTEGER            { Process(&@$, StatementTypeResolution, $2); }
    | TEMPO FLOAT                   { Process(&@$, StatementTypeTempo, $2); }
    | TEMPO INTEGER                 { Process(&@$, StatementTypeTempo, (float)$2); }
    | TIME INTEGER DIVISION INTEGER { Process(&@$, StatementTypeTimeSign, $2, $4); }
    | INTEGER COLON                 { Process(&@$, StatementTypeMeasure, $1); }
    | MARKER STRING                 { Process(&@$, StatementTypeMarker, $2); }
    | IDENTIFIER                    { Process(&@$, StatementTypePattern, $1); }
    | DEFINE IDENTIFIER             { Process(&@$, StatementTypePatternDefine, $2); }
    | END                           { Process(&@$, StatementTypeEnd, NULL); }
    | TRACK INTEGER                 { Process(&@$, StatementTypeTrack, $2); }
    | CHANNEL INTEGER               { Process(&@$, StatementTypeChannel, $2); }
    | VOICE INTEGER INTEGER INTEGER { Process(&@$, StatementTypeVoice, $2, $3, $4); }
    | SYNTH STRING                  { Process(&@$, StatementTypeSynth, $2); }
    | VOLUME INTEGER                { Process(&@$, StatementTypeVolume, $2); }
    | PAN INTEGER                   { Process(&@$, StatementTypePan, $2); }
    | PAN PLUS INTEGER              { Process(&@$, StatementTypePan, $3); }
    | PAN MINUS INTEGER             { Process(&@$, StatementTypePan, -$3); }
    | CHORUS INTEGER                { Process(&@$, StatementTypeChorus, $2); }
    | REVERB INTEGER                { Process(&@$, StatementTypeReverb, $2); }
    | TRANSPOSE INTEGER             { Process(&@$, StatementTypeTranspose, $2); }
    | TRANSPOSE PLUS INTEGER        { Process(&@$, StatementTypeTranspose, $3); }
    | TRANSPOSE MINUS INTEGER       { Process(&@$, StatementTypeTranspose, -$3); }
    | KEY KEY_SIGN                  { Process(&@$, StatementTypeKey, $2); }
    | NOTE                          { Process(&@$, StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS                    { Process(&@$, StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS            { Process(&@$, StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   MINUS    { Process(&@$, StatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   INTEGER  { Process(&@$, StatementTypeNote, $1, -1, -1, $4); }
    | NOTE MINUS   INTEGER          { Process(&@$, StatementTypeNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER MINUS    { Process(&@$, StatementTypeNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER INTEGER  { Process(&@$, StatementTypeNote, $1, -1, $3, $4); }
    | NOTE INTEGER                  { Process(&@$, StatementTypeNote, $1, $2, -1, -1); }
    | NOTE INTEGER INTEGER          { Process(&@$, StatementTypeNote, $1, $2, $3, -1); }
    | NOTE INTEGER INTEGER INTEGER  { Process(&@$, StatementTypeNote, $1, $2, $3, $4); }
    | NOTE INTEGER INTEGER MINUS    { Process(&@$, StatementTypeNote, $1, $2, $3, -1); }
    | NOTE INTEGER MINUS            { Process(&@$, StatementTypeNote, $1, $2, -1, -1); }
    | NOTE INTEGER MINUS   INTEGER  { Process(&@$, StatementTypeNote, $1, $2, -1, $4); }
    | NOTE INTEGER MINUS   MINUS    { Process(&@$, StatementTypeNote, $1, $2, -1, -1); }
    | MINUS INTEGER                 { Process(&@$, StatementTypeRest, $2); }
    | EOL
    | SEMICOLON
    | INCLUDE STRING                { Process(&@$, StatementTypeInclude, $2); }
    ;

%%

int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *message)
{
    NAMidiParser *parser = NAMidi_get_extra(scanner);
    NAMidiParserError(parser, yylloc->first_line, yylloc->first_column, ParseErrorKindSyntaxError);
    return 0;
}

static bool callProcess(YYLTYPE *yylloc, yyscan_t scanner, StatementType type, ...)
{
    NAMidiParser *parser = NAMidi_get_extra(scanner);

    va_list argList;
    va_start(argList, type);
    bool success = NAMidiParserProcess(parser, yylloc->first_line, yylloc->first_column, type, argList);
    va_end(argList);
    return success;
}

static void postProcess(YYLTYPE *yylloc, yyscan_t scanner, StatementType type, ...)
{
    va_list argList;

    switch (type) {
    case StatementTypeTitle:
    case StatementTypeMarker:
    case StatementTypePattern:
    case StatementTypePatternDefine:
    case StatementTypeSynth:
    case StatementTypeKey:
    case StatementTypeNote:
    case StatementTypeInclude:
        va_start(argList, type);
        free(va_arg(argList, char *));
        va_end(argList);
        break;
    default:
        break;
    }
}
