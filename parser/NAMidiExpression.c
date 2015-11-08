#include "NAMidiExpression.h"
#include "NoteTable.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define isValidRange(v, from, to) (from <= v && v <= to)
#define OCTAVE_NONE -99

static const char *STATEMENT_LIST_ID = "statement list";

typedef struct _StatementListExpr {
    Expression expr;
} StatementListExpr;

void *NAMidiExprStatementList(NAMidiParser *parser, const char *filepath, void *yylloc)
{
    return ExpressionCreate(filepath, yylloc, sizeof(StatementListExpr), STATEMENT_LIST_ID);
}

bool NAMidiExprIsStatementList(Expression *self)
{
    return self->identifier == STATEMENT_LIST_ID;
}

typedef struct _TitleExpr {
    Expression expr;
    char *title;
} TitleExpr;

static void TitleExprDestroy(void *_self)
{
    TitleExpr *self = _self;
    free(self->title);
    free(self);
}

void *NAMidiExprTitle(NAMidiParser *parser, const char *filepath, void *yylloc, const char *title)
{
    TitleExpr *self = ExpressionCreate(filepath, yylloc, sizeof(TitleExpr), "title");
    self->expr.vtbl.destroy = TitleExprDestroy;
    self->title = strdup(title);
    return self;
}

typedef struct _ResolutionExpr {
    Expression expr;
    int reslution;
} ResolutionExpr;

void *NAMidiExprResolution(NAMidiParser *parser, const char *filepath, void *yylloc, int reslution)
{
    ResolutionExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ResolutionExpr), "reslution");
    self->reslution = reslution;
    return self;
}

typedef struct _TempoExpr {
    Expression expr;
    float tempo;
} TempoExpr;

void *NAMidiExprTempo(NAMidiParser *parser, const char *filepath, void *yylloc, float tempo)
{
    TempoExpr *self = ExpressionCreate(filepath, yylloc, sizeof(TempoExpr), "tempo");
    self->tempo = tempo;
    return self;
}

typedef struct _TimeSignExpr {
    Expression expr;
    int numerator;
    int denominator;
} TimeSignExpr;

void *NAMidiExprTimeSign(NAMidiParser *parser, const char *filepath, void *yylloc, int numerator, int denominator)
{
    TimeSignExpr *self = ExpressionCreate(filepath, yylloc, sizeof(TimeSignExpr), "time sign");
    self->numerator = numerator;
    self->denominator = denominator;
    return self;
}

typedef struct _MarkerExpr {
    Expression expr;
    char *text;
} MarkerExpr;

static void MarkerExprDestroy(void *_self)
{
    MarkerExpr *self = _self;
    free(self->text);
    free(self);
}

void *NAMidiExprMarker(NAMidiParser *parser, const char *filepath, void *yylloc, const char *marker)
{
    MarkerExpr *self = ExpressionCreate(filepath, yylloc, sizeof(MarkerExpr), "marker");
    self->expr.vtbl.destroy = MarkerExprDestroy;
    self->text = strdup(marker);
    return self;
}

typedef struct _ChannelExpr {
    Expression expr;
    int channel;
} ChannelExpr;

void *NAMidiExprChannel(NAMidiParser *parser, const char *filepath, void *yylloc, int channel)
{
    ChannelExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ChannelExpr), "channel");
    self->channel = channel;
    return self;
}

typedef struct _VoiceExpr {
    Expression expr;
    int msb;
    int lsb;
    int programNo;
} VoiceExpr;

void *NAMidiExprVoice(NAMidiParser *parser, const char *filepath, void *yylloc, int msb, int lsb, int programNo)
{
    VoiceExpr *self = ExpressionCreate(filepath, yylloc, sizeof(VoiceExpr), "voice");
    self->msb = msb;
    self->lsb = lsb;
    self->programNo = programNo;
    return self;
}

typedef struct _SynthExpr {
    Expression expr;
    char *identifier;
} SynthExpr;

static void SynthExprDestroy(void *_self)
{
    SynthExpr *self = _self;
    free(self->identifier);
    free(self);
}

void *NAMidiExprSynth(NAMidiParser *parser, const char *filepath, void *yylloc, const char *identifier)
{
    SynthExpr *self = ExpressionCreate(filepath, yylloc, sizeof(SynthExpr), "synth");
    self->expr.vtbl.destroy = SynthExprDestroy;
    self->identifier = strdup(identifier);
    return self;
}

typedef struct _VolumeExpr {
    Expression expr;
    int value;
} VolumeExpr;

void *NAMidiExprVolume(NAMidiParser *parser, const char *filepath, void *yylloc, int value)
{
    VolumeExpr *self = ExpressionCreate(filepath, yylloc, sizeof(VolumeExpr), "volume");
    self->value = value;
    return self;
}

typedef VolumeExpr PanExpr;

void *NAMidiExprPan(NAMidiParser *parser, const char *filepath, void *yylloc, int value)
{
    PanExpr *self = ExpressionCreate(filepath, yylloc, sizeof(PanExpr), "pan");
    self->value = value;
    return self;
}

typedef VolumeExpr ChorusExpr;

void *NAMidiExprChorus(NAMidiParser *parser, const char *filepath, void *yylloc, int value)
{
    ChorusExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ChorusExpr), "chorus");
    self->value = value;
    return self;
}

typedef VolumeExpr ReverbExpr;

void *NAMidiExprReverb(NAMidiParser *parser, const char *filepath, void *yylloc, int value)
{
    ReverbExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ReverbExpr), "reverb");
    self->value = value;
    return self;
}

typedef VolumeExpr TransposeExpr;

void *NAMidiExprTranspose(NAMidiParser *parser, const char *filepath, void *yylloc, int value)
{
    TransposeExpr *self = ExpressionCreate(filepath, yylloc, sizeof(TransposeExpr), "transpose");
    self->value = value;
    return self;
}

typedef struct _KeySignExpr {
    Expression expr;
    KeySign keySign;
} KeySignExpr;

void *NAMidiExprKeySign(NAMidiParser *parser, const char *filepath, void *yylloc, const char *keyString)
{
    char keyChar = tolower(keyString[0]);
    bool sharp = NULL != strchr(keyString, '#');
    bool flat = NULL != strchr(keyString, 'b');
    bool major = NULL == strstr(keyString, "min");

    KeySign keySign = NoteTableGetKeySign(keyChar, sharp, flat, major);
    if (KeySignInvalid == keySign) {
        ParseLocation *location = yylloc;
        NAMidiParserError(parser, filepath, location->line, location->column, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    KeySignExpr *self = ExpressionCreate(filepath, yylloc, sizeof(KeySignExpr), "key sign");
    self->keySign = keySign;
    return self;
}

typedef struct _RestExpr {
    Expression expr;
    int step;
} RestExpr;

void *NAMidiExprRest(NAMidiParser *parser, const char *filepath, void *yylloc, int step)
{
    RestExpr *self = ExpressionCreate(filepath, yylloc, sizeof(RestExpr), "rest");
    self->step = step;
    return self;
}

typedef struct _NoteExpr {
    Expression expr;
    int step;
    BaseNote baseNote;
    Accidental accidental;
    int octave;
    int gatetime;
    int velocity;
} NoteExpr;

void *NAMidiExprNote(NAMidiParser *parser, const char *filepath, void *yylloc, const char *noteString, int step, int gatetime, int velocity)
{
    const BaseNote noteTable[] = {
        BaseNote_A, BaseNote_B, BaseNote_C,
        BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
    };

    const char *pc = noteString;

    BaseNote baseNote = noteTable[tolower(*pc) - 97];
    Accidental accidental = AccidentalNone;

    int octave = OCTAVE_NONE;
    const char *c;
    while (*(c = ++pc)) {
        switch (*c) {
        case '#':
            accidental = AccidentalSharp == accidental ? AccidentalDoubleSharp : AccidentalSharp;
            break;
        case 'b':
            accidental = AccidentalFlat == accidental ? AccidentalDoubleFlat : AccidentalFlat;
            break;
        case 'n':
            accidental = AccidentalNatural;
            break;
        default:
            octave = atoi(c);
            break;
        }

        if (OCTAVE_NONE != octave) {
            break;
        }
    }

    if (!isValidRange(step, -1, 65535)
            || !isValidRange(gatetime, -1, 65535)
            || !isValidRange(velocity, -1, 127)
            || (OCTAVE_NONE != octave && !isValidRange(octave, -2, 8))) {
        ParseLocation *location = yylloc;
        NAMidiParserError(parser, filepath, location->line, location->column, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    NoteExpr *self = ExpressionCreate(filepath, yylloc, sizeof(NoteExpr), "note");
    self->step = step;
    self->baseNote = baseNote;
    self->accidental = accidental;
    self->octave = octave;
    self->gatetime = gatetime;
    self->velocity = velocity;
    return self;
}

void *NAMidiExprPattern(NAMidiParser *parser, const char *filepath, void *yylloc, const char *identifier, Expression *statementList)
{
}

void *NAMidiExprPatternExpand(NAMidiParser *parser, const char *filepath, void *yylloc, const char *identifier, NAArray *idList);
void *NAMidiExprContext(NAMidiParser *parser, const char *filepath, void *yylloc, NAArray *idList, Expression *statementList);
