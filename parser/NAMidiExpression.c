#include "NAMidiExpression.h"

#include <stdlib.h>
#include <string.h>

static const char *STATEMENT_LIST_ID = "statement list";

typedef struct _StatementListExpr {
    Expression expr;
} StatementListExpr;

void *NAMidiExprStatementList(const char *filepath, void *yylloc)
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

void *NAMidiExprTitle(const char *filepath, void *yylloc, const char *title)
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

void *NAMidiExprResolution(const char *filepath, void *yylloc, int reslution)
{
    ResolutionExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ResolutionExpr), "reslution");
    self->reslution = reslution;
    return self;
}

typedef struct _TempoExpr {
    Expression expr;
    float tempo;
} TempoExpr;

void *NAMidiExprTempo(const char *filepath, void *yylloc, float tempo)
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

void *NAMidiExprTimeSign(const char *filepath, void *yylloc, int numerator, int denominator)
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

void *NAMidiExprMarker(const char *filepath, void *yylloc, const char *marker)
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

void *NAMidiExprChannel(const char *filepath, void *yylloc, int channel)
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

void *NAMidiExprVoice(const char *filepath, void *yylloc, int msb, int lsb, int programNo)
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

void *NAMidiExprSynth(const char *filepath, void *yylloc, const char *identifier)
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

void *NAMidiExprVolume(const char *filepath, void *yylloc, int value)
{
    VolumeExpr *self = ExpressionCreate(filepath, yylloc, sizeof(VolumeExpr), "volume");
    self->value = value;
    return self;
}

typedef VolumeExpr PanExpr;

void *NAMidiExprPan(const char *filepath, void *yylloc, int value)
{
    PanExpr *self = ExpressionCreate(filepath, yylloc, sizeof(PanExpr), "pan");
    self->value = value;
    return self;
}

typedef VolumeExpr ChorusExpr;

void *NAMidiExprChorus(const char *filepath, void *yylloc, int value)
{
    ChorusExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ChorusExpr), "chorus");
    self->value = value;
    return self;
}

typedef VolumeExpr ReverbExpr;

void *NAMidiExprReverb(const char *filepath, void *yylloc, int value)
{
    ReverbExpr *self = ExpressionCreate(filepath, yylloc, sizeof(ReverbExpr), "reverb");
    self->value = value;
    return self;
}

typedef VolumeExpr TransposeExpr;

void *NAMidiExprTranspose(const char *filepath, void *yylloc, int value)
{
    TransposeExpr *self = ExpressionCreate(filepath, yylloc, sizeof(TransposeExpr), "transpose");
    self->value = value;
    return self;
}

void *NAMidiExprKeySign(const char *filepath, void *yylloc, const char *keySign)
{
}
void *NAMidiExprRest(const char *filepath, void *yylloc, int step);
void *NAMidiExprNote(const char *filepath, void *yylloc, const char *noteString, int step, int gatetime, int velocity);
void *NAMidiExprPattern(const char *filepath, void *yylloc, const char *identifier, Expression *statementList);
void *NAMidiExprPatternExpand(const char *filepath, void *yylloc, const char *identifier, NAArray *idList);
void *NAMidiExprContext(const char *filepath, void *yylloc, NAArray *idList, Expression *statementList);
