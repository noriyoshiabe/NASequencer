%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"
#include "NAMidiParser.h"

#include <ctype.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *message);

static bool callProcess(YYLTYPE *yylloc, yyscan_t scanner, NAMidiStatementType type, ...);
static void postProcess(YYLTYPE *yylloc, yyscan_t scanner, NAMidiStatementType type, ...);

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
    : TITLE STRING                  { Process(&@$, NAMidiStatementTypeTitle, $2); }
    | RESOLUTION INTEGER            { Process(&@$, NAMidiStatementTypeResolution, $2); }
    | TEMPO FLOAT                   { Process(&@$, NAMidiStatementTypeTempo, $2); }
    | TEMPO INTEGER                 { Process(&@$, NAMidiStatementTypeTempo, (float)$2); }
    | TIME INTEGER DIVISION INTEGER { Process(&@$, NAMidiStatementTypeTimeSign, $2, $4); }
    | MARKER STRING                 { Process(&@$, NAMidiStatementTypeMarker, $2); }
    | IDENTIFIER                    { Process(&@$, NAMidiStatementTypePattern, $1, NULL); }
    | IDENTIFIER LPAREN IDENTIFIER RPAREN
                                    { Process(&@$, NAMidiStatementTypePattern, $1, $3); }
    | DEFINE IDENTIFIER             { Process(&@$, NAMidiStatementTypePatternDefine, $2); }
    | IDENTIFIER LCURLY             { Process(&@$, NAMidiStatementTypeContext, $1); }
    | DEFAULT LCURLY                { Process(&@$, NAMidiStatementTypeContextDefault, NULL); }
    | RCURLY                        { Process(&@$, NAMidiStatementTypeContextEnd, NULL); }
    | END                           { Process(&@$, NAMidiStatementTypeEnd, NULL); }
    | CHANNEL INTEGER               { Process(&@$, NAMidiStatementTypeChannel, $2); }
    | VOICE INTEGER INTEGER INTEGER { Process(&@$, NAMidiStatementTypeVoice, $2, $3, $4); }
    | SYNTH STRING                  { Process(&@$, NAMidiStatementTypeSynth, $2); }
    | VOLUME INTEGER                { Process(&@$, NAMidiStatementTypeVolume, $2); }
    | PAN INTEGER                   { Process(&@$, NAMidiStatementTypePan, $2); }
    | PAN PLUS INTEGER              { Process(&@$, NAMidiStatementTypePan, $3); }
    | PAN MINUS INTEGER             { Process(&@$, NAMidiStatementTypePan, -$3); }
    | CHORUS INTEGER                { Process(&@$, NAMidiStatementTypeChorus, $2); }
    | REVERB INTEGER                { Process(&@$, NAMidiStatementTypeReverb, $2); }
    | TRANSPOSE INTEGER             { Process(&@$, NAMidiStatementTypeTranspose, $2); }
    | TRANSPOSE PLUS INTEGER        { Process(&@$, NAMidiStatementTypeTranspose, $3); }
    | TRANSPOSE MINUS INTEGER       { Process(&@$, NAMidiStatementTypeTranspose, -$3); }
    | KEY KEY_SIGN                  { Process(&@$, NAMidiStatementTypeKey, $2); }
    | NOTE                          { Process(&@$, NAMidiStatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS                    { Process(&@$, NAMidiStatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS            { Process(&@$, NAMidiStatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   MINUS    { Process(&@$, NAMidiStatementTypeNote, $1, -1, -1, -1); }
    | NOTE MINUS   MINUS   INTEGER  { Process(&@$, NAMidiStatementTypeNote, $1, -1, -1, $4); }
    | NOTE MINUS   INTEGER          { Process(&@$, NAMidiStatementTypeNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER MINUS    { Process(&@$, NAMidiStatementTypeNote, $1, -1, $3, -1); }
    | NOTE MINUS   INTEGER INTEGER  { Process(&@$, NAMidiStatementTypeNote, $1, -1, $3, $4); }
    | NOTE INTEGER                  { Process(&@$, NAMidiStatementTypeNote, $1, $2, -1, -1); }
    | NOTE INTEGER INTEGER          { Process(&@$, NAMidiStatementTypeNote, $1, $2, $3, -1); }
    | NOTE INTEGER INTEGER INTEGER  { Process(&@$, NAMidiStatementTypeNote, $1, $2, $3, $4); }
    | NOTE INTEGER INTEGER MINUS    { Process(&@$, NAMidiStatementTypeNote, $1, $2, $3, -1); }
    | NOTE INTEGER MINUS            { Process(&@$, NAMidiStatementTypeNote, $1, $2, -1, -1); }
    | NOTE INTEGER MINUS   INTEGER  { Process(&@$, NAMidiStatementTypeNote, $1, $2, -1, $4); }
    | NOTE INTEGER MINUS   MINUS    { Process(&@$, NAMidiStatementTypeNote, $1, $2, -1, -1); }
    | MINUS INTEGER                 { Process(&@$, NAMidiStatementTypeRest, $2); }
    | EOL
    | SEMICOLON
    | INCLUDE STRING                { Process(&@$, NAMidiStatementTypeInclude, $2); }
    ;

%%

int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *message)
{
    NAMidiParser *parser = NAMidi_get_extra(scanner);
    NAMidiParserError(parser, yylloc->first_line, yylloc->first_column, ParseErrorKindSyntaxError);
    return 0;
}

static bool callProcess(YYLTYPE *yylloc, yyscan_t scanner, NAMidiStatementType type, ...)
{
    NAMidiParser *parser = NAMidi_get_extra(scanner);

    va_list argList;
    va_start(argList, type);
    bool success = NAMidiParserProcess(parser, yylloc->first_line, yylloc->first_column, type, argList);
    va_end(argList);
    return success;
}

static void postProcess(YYLTYPE *yylloc, yyscan_t scanner, NAMidiStatementType type, ...)
{
    va_list argList;

    switch (type) {
    case NAMidiStatementTypeTitle:
    case NAMidiStatementTypeMarker:
    case NAMidiStatementTypePatternDefine:
    case NAMidiStatementTypeContext:
    case NAMidiStatementTypeSynth:
    case NAMidiStatementTypeKey:
    case NAMidiStatementTypeNote:
    case NAMidiStatementTypeInclude:
        va_start(argList, type);
        free(va_arg(argList, char *));
        va_end(argList);
        break;
    case NAMidiStatementTypePattern:
        va_start(argList, type);
        free(va_arg(argList, char *));
        {
            char *p = va_arg(argList, char *);
            if (p) {
                free(p);
            }
        }
        va_end(argList);
        break;
    default:
        break;
    }
}
