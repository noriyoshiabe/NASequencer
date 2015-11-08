#include "NAMidiExpression.h"
#include "NoteTable.h"
#include "NAMap.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define isValidRange(v, from, to) (from <= v && v <= to)
#define OCTAVE_NONE -99

#if 0
#define __Trace__ printf("%s - %s:%d:%d\n", __func__, filepath, ((int *)yylloc)[0], ((int *)yylloc)[1]);
#else
#define __Trace__
#endif

static const char *STATEMENT_LIST_ID = "statement list";
static const char *PATTERN_ID = "pattern";

typedef struct _StatementListExpr {
    Expression expr;
    NAMap *patternMap;
} StatementListExpr;

static void StatementListExprDestroy(void *_self)
{
    StatementListExpr *self = _self;
    NAMapDestroy(self->patternMap);
    free(self);
}

void *NAMidiExprStatementList(NAMidiParser *parser, const char *filepath, void *yylloc)
{ __Trace__
    StatementListExpr *self = ExpressionCreate(filepath, yylloc, sizeof(StatementListExpr), STATEMENT_LIST_ID);
    self->expr.vtbl.destroy = StatementListExprDestroy;
    self->patternMap = NAMapCreate(NAHashCString, NADescriptionCString, NULL);
    return self;
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
{ __Trace__
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
{ __Trace__
    ResolutionExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ResolutionExpr), "reslution");
    self->reslution = reslution;
    return self;
}

typedef struct _TempoExpr {
    Expression expr;
    float tempo;
} TempoExpr;

void *NAMidiExprTempo(NAMidiParser *parser, const char *filepath, void *yylloc, float tempo)
{ __Trace__
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
{ __Trace__
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
{ __Trace__
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
{ __Trace__
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
{ __Trace__
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
{ __Trace__
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
{ __Trace__
    VolumeExpr *self = ExpressionCreate(filepath, yylloc, sizeof(VolumeExpr), "volume");
    self->value = value;
    return self;
}

typedef VolumeExpr PanExpr;

void *NAMidiExprPan(NAMidiParser *parser, const char *filepath, void *yylloc, int value)
{ __Trace__
    PanExpr *self = ExpressionCreate(filepath, yylloc, sizeof(PanExpr), "pan");
    self->value = value;
    return self;
}

typedef VolumeExpr ChorusExpr;

void *NAMidiExprChorus(NAMidiParser *parser, const char *filepath, void *yylloc, int value)
{ __Trace__
    ChorusExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ChorusExpr), "chorus");
    self->value = value;
    return self;
}

typedef VolumeExpr ReverbExpr;

void *NAMidiExprReverb(NAMidiParser *parser, const char *filepath, void *yylloc, int value)
{ __Trace__
    ReverbExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ReverbExpr), "reverb");
    self->value = value;
    return self;
}

typedef VolumeExpr TransposeExpr;

void *NAMidiExprTranspose(NAMidiParser *parser, const char *filepath, void *yylloc, int value)
{ __Trace__
    TransposeExpr *self = ExpressionCreate(filepath, yylloc, sizeof(TransposeExpr), "transpose");
    self->value = value;
    return self;
}

typedef struct _KeySignExpr {
    Expression expr;
    KeySign keySign;
} KeySignExpr;

void *NAMidiExprKeySign(NAMidiParser *parser, const char *filepath, void *yylloc, const char *keyString)
{ __Trace__
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
{ __Trace__
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
{ __Trace__
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

typedef struct _PatternExpr {
    Expression expr;
    char *identifier;
} PatternExpr;

static void PatternExprDestroy(void *_self)
{
    PatternExpr *self = _self;
    free(self->identifier);
    free(self);
}

void *NAMidiExprPattern(NAMidiParser *parser, const char *filepath, void *yylloc, const char *identifier, Expression *statementList)
{ __Trace__
    PatternExpr *self = ExpressionCreate(filepath, yylloc, sizeof(PatternExpr), PATTERN_ID);
    self->expr.vtbl.destroy = PatternExprDestroy;
    self->identifier = strdup(identifier);
    ExpressionAddChild(&self->expr, statementList);
    return self;
}

typedef struct _PatternExpandExpr {
    Expression expr;
    char *identifier;
    NAArray *contextIdList;
} PatternExpandExpr;

static void PatternExprExpandDestroy(void *_self)
{
    PatternExpandExpr *self = _self;
    free(self->identifier);
    NAArrayTraverse(self->contextIdList, free);
    NAArrayDestroy(self->contextIdList);
    free(self);
}

void *NAMidiExprPatternExpand(NAMidiParser *parser, const char *filepath, void *yylloc, const char *identifier, NAArray *idList)
{ __Trace__
    PatternExpandExpr *self = ExpressionCreate(filepath, yylloc, sizeof(PatternExpandExpr), "pattern expand");
    self->expr.vtbl.destroy = PatternExprExpandDestroy;
    self->identifier = strdup(identifier);
    self->contextIdList = idList;
    return self;
}

typedef struct _ContextExpr {
    Expression expr;
    NAArray *contextIdList;
} ContextExpr;

static void ContextExprExpandDestroy(void *_self)
{
    ContextExpr *self = _self;
    NAArrayTraverse(self->contextIdList, free);
    NAArrayDestroy(self->contextIdList);
    free(self);
}

void *NAMidiExprContext(NAMidiParser *parser, const char *filepath, void *yylloc, NAArray *idList, Expression *statementList)
{ __Trace__
    ContextExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ContextExpr), "context");
    self->expr.vtbl.destroy = ContextExprExpandDestroy;
    self->contextIdList = idList;
    ExpressionAddChild(&self->expr, statementList);
    return self;
}

bool NAMidiExprIsStatementList(Expression *self)
{
    return self->identifier == STATEMENT_LIST_ID;
}

bool NAMidiExprIsPattern(Expression *self)
{
    return self->identifier == PATTERN_ID;
}

void NAMidiExprStatementListAddPattern(void *_self, void *_pattern)
{
    StatementListExpr *self = _self;
    PatternExpr *pattern = _pattern;
    NAMapPut(self->patternMap, pattern->identifier, pattern);
}
