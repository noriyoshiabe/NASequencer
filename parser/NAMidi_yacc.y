%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"
#include "Parser.h"

#include <ctype.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *message);

static bool callbackStatement(YYLTYPE *yylloc, yyscan_t scanner, StatementType type, ...);
static void postProcess(yyscan_t scanner, StatementType type, ...);

#define CALLBACK(yylloc, type, ...) \
    bool success = callbackStatement(yylloc, scanner, type, __VA_ARGS__); \
    postProcess(scanner, type, __VA_ARGS__); \
    if (!success) { \
        return 1;\
    }

#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))
#define isValidRange(v, from, to) (from <= v && v <= to)

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
    : TITLE STRING                  { CALLBACK(&@$, StatementTypeTitle, $2); }
    | RESOLUTION INTEGER            { CALLBACK(&@$, StatementTypeResolution, $2); }
    | TEMPO FLOAT                   { CALLBACK(&@$, StatementTypeTempo, $2); }
    | TEMPO INTEGER                 { CALLBACK(&@$, StatementTypeTempo, (float)$2); }
    | TIME INTEGER DIVISION INTEGER { CALLBACK(&@$, StatementTypeTimeSign, $2, $4); }
    | INTEGER COLON                 { CALLBACK(&@$, StatementTypeMeasure, $1); }
    | MARKER STRING                 { CALLBACK(&@$, StatementTypeMarker, $2); }
    | IDENTIFIER                    { CALLBACK(&@$, StatementTypePattern, $1); }
    | DEFINE IDENTIFIER             { CALLBACK(&@$, StatementTypePatternDefine, $2); }
    | END                           { CALLBACK(&@$, StatementTypeEnd, NULL); }
    | TRACK INTEGER                 { CALLBACK(&@$, StatementTypeTrack, $2); }
    | CHANNEL INTEGER               { CALLBACK(&@$, StatementTypeChannel, $2); }
    | VOICE INTEGER INTEGER INTEGER { CALLBACK(&@$, StatementTypeVoice, $2, $3, $4); }
    | VOLUME INTEGER                { CALLBACK(&@$, StatementTypeVolume, $2); }
    | PAN INTEGER                   { CALLBACK(&@$, StatementTypePan, $2); }
    | PAN PLUS INTEGER              { CALLBACK(&@$, StatementTypePan, $3); }
    | PAN MINUS INTEGER             { CALLBACK(&@$, StatementTypePan, -$3); }
    | CHORUS INTEGER                { CALLBACK(&@$, StatementTypeChorus, $2); }
    | REVERB INTEGER                { CALLBACK(&@$, StatementTypeReverb, $2); }
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
    | INCLUDE STRING                { CALLBACK(&@$, StatementTypeInclude, $2); }
    ;

%%

int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *message)
{
    ParseContext *context = NAMidi_get_extra(scanner);
    context->location.line = yylloc->first_line;
    context->location.column = yylloc->first_column;

    context->result->error.kind = ParseErrorKindSyntaxError;
    context->result->error.location = context->location;

    context->handler->error(context->receiver, context, &context->result->error);
    return 0;
}

static void callbackError(ParseContext *context, ParseErrorKind kind)
{
    context->result->error.kind = kind;
    context->result->error.location = context->location;
    context->handler->error(context->receiver, context, &context->result->error);
}

static bool callbackStatement(YYLTYPE *yylloc, yyscan_t scanner, StatementType type, ...)
{
    ParseContext *context = NAMidi_get_extra(scanner);
    context->location.line = yylloc->first_line;
    context->location.column = yylloc->first_column;

    bool success = false;

    va_list argList;
    va_start(argList, type);

    switch (type) {
    case StatementTypeResolution:
        {
            int resolution = va_arg(argList, int);
            if (!isValidRange(resolution, 1, 9600)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, resolution);
            }
        }
        break;
    case StatementTypeTitle:
        success = context->handler->process(context->receiver, context, type, va_arg(argList, char *));
        break;
    case StatementTypeTempo:
        {
            double tempo = va_arg(argList, double);
            if (!isValidRange(tempo, 30.0, 300.0)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, tempo);
            }
        }
        break;
    case StatementTypeTimeSign:
        {
            int numerator = va_arg(argList, int);
            int denominator = va_arg(argList, int);
            if (1 > numerator || 1 > denominator || !isPowerOf2(denominator)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, numerator, denominator);
            }
        }
        break;
    case StatementTypeMeasure:
        {
            int measure = va_arg(argList, int);
            if (!isValidRange(measure, 1, ParserMeasureMax)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, measure);
            }
        }
        break;
    case StatementTypeMarker:
        success = context->handler->process(context->receiver, context, type, va_arg(argList, char *));
        break;
    case StatementTypePattern:
        success = context->handler->process(context->receiver, context, type, va_arg(argList, char *));
        break;
    case StatementTypePatternDefine:
        success = context->handler->process(context->receiver, context, type, va_arg(argList, char *));
        break;
    case StatementTypeEnd:
        success = context->handler->process(context->receiver, context, type);
        break;
    case StatementTypeTrack:
        {
            int track = va_arg(argList, int);
            if (!isValidRange(track, 1, 16)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, track);
            }
        }
        break;
    case StatementTypeChannel:
        {
            int channel = va_arg(argList, int);
            if (!isValidRange(channel, 1, 16)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, channel);
            }
        }
        break;
    case StatementTypeVoice:
        {
            int msb = va_arg(argList, int);
            int lsb = va_arg(argList, int);
            int programNo = va_arg(argList, int);

            if (!isValidRange(msb, 0, 127)
                    || !isValidRange(lsb, 0, 127)
                    || !isValidRange(programNo, 0, 127)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, msb, lsb, programNo);
            }
        }
        break;
    case StatementTypeVolume:
        {
            int volume = va_arg(argList, int);
            if (!isValidRange(volume, 0, 127)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, volume);
            }
        }
        break;
    case StatementTypePan:
        {
            int pan = va_arg(argList, int);
            if (!isValidRange(pan, -64, 64)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, pan);
            }
        }
        break;
    case StatementTypeChorus:
        {
            int chorus = va_arg(argList, int);
            if (!isValidRange(chorus, 0, 127)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, chorus);
            }
        }
        break;
    case StatementTypeReverb:
        {
            int reverb = va_arg(argList, int);
            if (!isValidRange(reverb, 0, 127)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, reverb);
            }
        }
        break;
    case StatementTypeTranspose:
        {
            int transpose = va_arg(argList, int);
            if (!isValidRange(transpose, -64, 64)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, transpose);
            }
        }
        break;
    case StatementTypeKey:
        {
            char *keyString = va_arg(argList, char *);

            char keyChar = tolower(keyString[0]);
            bool sharp = NULL != strchr(keyString, '#');
            bool flat = NULL != strchr(keyString, 'b');
            bool major = NULL == strstr(keyString, "min");

            KeySign keySign = NoteTableGetKeySign(keyChar, sharp, flat, major);
            if (KeySignInvalid == keySign) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
              success = context->handler->process(context->receiver, context, type, keySign);
            }
        }
        break;
    case StatementTypeNote:
        {
            const BaseNote noteTable[] = {
                BaseNote_A, BaseNote_B, BaseNote_C,
                BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
            };

            char *pc = va_arg(argList, char *);

            BaseNote baseNote = noteTable[tolower(*pc) - 97];
            Accidental accidental = AccidentalNone;

            switch (*(++pc)) {
            case '#':
                accidental = AccidentalSharp;
                ++pc;
                break;
            case 'b':
                accidental = AccidentalFlat;
                ++pc;
                break;
            case 'n':
                accidental = AccidentalNatural;
                ++pc;
                break;
            }

            int octave = atoi(pc);

            int step = va_arg(argList, int);
            int gatetime = va_arg(argList, int);
            int velocity = va_arg(argList, int);

            if (!isValidRange(step, -1, 65535)
                    || !isValidRange(gatetime, -1, 65535)
                    || !isValidRange(velocity, -1, 127)
                    || !isValidRange(octave, -2, 8)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type,
                    baseNote, accidental, octave, step, gatetime, velocity);
            }
        }
        break;
    case StatementTypeRest:
        {
            int step = va_arg(argList, int);
            if (!isValidRange(step, 0, 65535)) {
                callbackError(context, ParseErrorKindInvalidValue);
            }
            else {
                success = context->handler->process(context->receiver, context, type, step);
            }
        }
        break;
    case StatementTypeInclude:
        success = context->handler->process(context->receiver, context, type, va_arg(argList, char *));
        break;
    default:
        break;
    }

    va_end(argList);
    return success;
}

static void postProcess(yyscan_t scanner, StatementType type, ...)
{
    va_list argList;

    switch (type) {
    case StatementTypeTitle:
    case StatementTypeMarker:
    case StatementTypePattern:
    case StatementTypePatternDefine:
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
