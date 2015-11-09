#include "NAMidiExpression.h"
#include "NoteTable.h"
#include "NAUtil.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))
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

static bool StatementListExprParse(void *_self, void *parser, void *_context)
{
    StatementListExpr *self = _self;
    NAMidiParserContext *context = _context ? NAMidiParserContextCreateCopy(_context) : NAMidiParserContextCreate();
    context->patternMap = self->patternMap;

    bool success = true;

    int count = NAArrayCount(self->expr.children);
    void **values = NAArrayGetValues(self->expr.children);
    for (int i = 0; i < count; ++i) {
        if (!ExpressionParse(values[i], parser, context)) {
            success = false;
            goto ERROR;
        }
    }

    if (!_context) {
        SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
        builder->setLength(builder, NAMidiParserContextGetLength(context));
    }

ERROR:
    NAMidiParserContextDestroy(context);
    return success;
}

void *NAMidiExprStatementList(NAMidiParser *parser, ParseLocation *location)
{ __Trace__
    StatementListExpr *self = ExpressionCreate(location, sizeof(StatementListExpr), STATEMENT_LIST_ID);
    self->expr.vtbl.destroy = StatementListExprDestroy;
    self->expr.vtbl.parse = StatementListExprParse;
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

static bool TitleExprParse(void *_self, void *parser, void *_context)
{
    TitleExpr *self = _self;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->setTitle(builder, self->title);
    return true;
}

void *NAMidiExprTitle(NAMidiParser *parser, ParseLocation *location, char *title)
{ __Trace__
    TitleExpr *self = ExpressionCreate(location, sizeof(TitleExpr), "title");
    self->expr.vtbl.destroy = TitleExprDestroy;
    self->expr.vtbl.parse = TitleExprParse;
    self->title = title;
    return self;
}

typedef struct _ResolutionExpr {
    Expression expr;
    int resolution;
} ResolutionExpr;

static bool ResolutionExprParse(void *_self, void *parser, void *_context)
{
    ResolutionExpr *self = _self;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->setResolution(builder, self->resolution);
    return true;
}

void *NAMidiExprResolution(NAMidiParser *parser, ParseLocation *location, int resolution)
{ __Trace__
    if (!isValidRange(resolution, 1, 9600)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    ResolutionExpr *self = ExpressionCreate(location, sizeof(ResolutionExpr), "reslution");
    self->expr.vtbl.parse = ResolutionExprParse;
    self->resolution = resolution;
    return self;
}

typedef struct _TempoExpr {
    Expression expr;
    float tempo;
} TempoExpr;

static bool TempoExprParse(void *_self, void *parser, void *_context)
{
    TempoExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->appendTempo(builder, ++context->id, context->channels[context->channel].tick, self->tempo);
    return true;
}

void *NAMidiExprTempo(NAMidiParser *parser, ParseLocation *location, float tempo)
{ __Trace__
    if (!isValidRange(tempo, 30.0, 300.0)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    TempoExpr *self = ExpressionCreate(location, sizeof(TempoExpr), "tempo");
    self->expr.vtbl.parse = TempoExprParse;
    self->tempo = tempo;
    return self;
}

typedef struct _TimeSignExpr {
    Expression expr;
    int numerator;
    int denominator;
} TimeSignExpr;

static bool TimeSignExprParse(void *_self, void *parser, void *_context)
{
    TimeSignExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->appendTimeSign(builder, ++context->id, context->channels[context->channel].tick, self->numerator, self->denominator);
    return true;
}

void *NAMidiExprTimeSign(NAMidiParser *parser, ParseLocation *location, int numerator, int denominator)
{ __Trace__
    if (1 > numerator || 1 > denominator || !isPowerOf2(denominator)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    TimeSignExpr *self = ExpressionCreate(location, sizeof(TimeSignExpr), "time sign");
    self->expr.vtbl.parse = TimeSignExprParse;
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

static bool MarkerExprParse(void *_self, void *parser, void *_context)
{
    MarkerExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->appendMarker(builder, ++context->id, context->channels[context->channel].tick, self->text);
    return true;
}

void *NAMidiExprMarker(NAMidiParser *parser, ParseLocation *location, char *marker)
{ __Trace__
    MarkerExpr *self = ExpressionCreate(location, sizeof(MarkerExpr), "marker");
    self->expr.vtbl.destroy = MarkerExprDestroy;
    self->expr.vtbl.parse = MarkerExprParse;
    self->text = marker;
    return self;
}

typedef struct _ChannelExpr {
    Expression expr;
    int channel;
} ChannelExpr;

static bool ChannelExprParse(void *_self, void *parser, void *_context)
{
    ChannelExpr *self = _self;
    NAMidiParserContext *context = _context;
    context->channel = self->channel;
    return true;
}

void *NAMidiExprChannel(NAMidiParser *parser, ParseLocation *location, int channel)
{ __Trace__
    if (!isValidRange(channel, 1, 16)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    ChannelExpr *self = ExpressionCreate(location, sizeof(ChannelExpr), "channel");
    self->expr.vtbl.parse = ChannelExprParse;
    self->channel = channel;
    return self;
}

typedef struct _VoiceExpr {
    Expression expr;
    int msb;
    int lsb;
    int programNo;
} VoiceExpr;

static bool VoiceExprParse(void *_self, void *parser, void *_context)
{
    VoiceExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->appendVoice(builder, ++context->id, context->channels[context->channel].tick, context->channel, self->msb, self->lsb, self->programNo);
    return true;
}

void *NAMidiExprVoice(NAMidiParser *parser, ParseLocation *location, int msb, int lsb, int programNo)
{ __Trace__
    if (!isValidRange(msb, 0, 127) || !isValidRange(lsb, 0, 127) || !isValidRange(programNo, 0, 127)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    VoiceExpr *self = ExpressionCreate(location, sizeof(VoiceExpr), "voice");
    self->expr.vtbl.parse = VoiceExprParse;
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

static bool SynthExprParse(void *_self, void *parser, void *_context)
{
    SynthExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->appendSynth(builder, ++context->id, context->channels[context->channel].tick, context->channel, self->identifier);
    return true;
}

void *NAMidiExprSynth(NAMidiParser *parser, ParseLocation *location, char *identifier)
{ __Trace__
    SynthExpr *self = ExpressionCreate(location, sizeof(SynthExpr), "synth");
    self->expr.vtbl.destroy = SynthExprDestroy;
    self->expr.vtbl.parse = SynthExprParse;
    self->identifier = identifier;
    return self;
}

typedef struct _VolumeExpr {
    Expression expr;
    int value;
} VolumeExpr;

static bool VolumeExprParse(void *_self, void *parser, void *_context)
{
    VolumeExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->appendVolume(builder, ++context->id, context->channels[context->channel].tick, context->channel, self->value);
    return true;
}

void *NAMidiExprVolume(NAMidiParser *parser, ParseLocation *location, int value)
{ __Trace__
    if (!isValidRange(value, 0, 127)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
    }

    VolumeExpr *self = ExpressionCreate(location, sizeof(VolumeExpr), "volume");
    self->expr.vtbl.parse = VolumeExprParse;
    self->value = value;
    return self;
}

typedef VolumeExpr PanExpr;

static bool PanExprParse(void *_self, void *parser, void *_context)
{
    PanExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->appendPan(builder, ++context->id, context->channels[context->channel].tick, context->channel, self->value);
    return true;
}

void *NAMidiExprPan(NAMidiParser *parser, ParseLocation *location, int value)
{ __Trace__
    if (!isValidRange(value, -64, 64)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
    }

    PanExpr *self = ExpressionCreate(location, sizeof(PanExpr), "pan");
    self->expr.vtbl.parse = PanExprParse;
    self->value = value;
    return self;
}

typedef VolumeExpr ChorusExpr;

static bool ChorusExprParse(void *_self, void *parser, void *_context)
{
    ChorusExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->appendChorus(builder, ++context->id, context->channels[context->channel].tick, context->channel, self->value);
    return true;
}

void *NAMidiExprChorus(NAMidiParser *parser, ParseLocation *location, int value)
{ __Trace__
    if (!isValidRange(value, 0, 127)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
    }

    ChorusExpr *self = ExpressionCreate(location, sizeof(ChorusExpr), "chorus");
    self->expr.vtbl.parse = ChorusExprParse;
    self->value = value;
    return self;
}

typedef VolumeExpr ReverbExpr;

static bool ReverbExprParse(void *_self, void *parser, void *_context)
{
    ReverbExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);
    builder->appendReverb(builder, ++context->id, context->channels[context->channel].tick, context->channel, self->value);
    return true;
}

void *NAMidiExprReverb(NAMidiParser *parser, ParseLocation *location, int value)
{ __Trace__
    if (!isValidRange(value, 0, 127)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
    }

    ReverbExpr *self = ExpressionCreate(location, sizeof(ReverbExpr), "reverb");
    self->expr.vtbl.parse = ReverbExprParse;
    self->value = value;
    return self;
}

typedef VolumeExpr TransposeExpr;

static bool TransposeExprParse(void *_self, void *parser, void *_context)
{
    TransposeExpr *self = _self;
    NAMidiParserContext *context = _context;
    context->transpose = self->value;
    return true;
}

void *NAMidiExprTranspose(NAMidiParser *parser, ParseLocation *location, int value)
{ __Trace__
    if (!isValidRange(value, -64, 64)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
    }

    TransposeExpr *self = ExpressionCreate(location, sizeof(TransposeExpr), "transpose");
    self->expr.vtbl.parse = TransposeExprParse;
    self->value = value;
    return self;
}

typedef struct _KeySignExpr {
    Expression expr;
    KeySign keySign;
} KeySignExpr;

static bool KeySignExprParse(void *_self, void *parser, void *_context)
{
    KeySignExpr *self = _self;
    NAMidiParserContext *context = _context;
    context->keySign = self->keySign;
    return true;
}

void *NAMidiExprKeySign(NAMidiParser *parser, ParseLocation *location, char *keyString)
{ __Trace__
    char keyChar = tolower(keyString[0]);
    bool sharp = NULL != strchr(keyString, '#');
    bool flat = NULL != strchr(keyString, 'b');
    bool major = NULL == strstr(keyString, "min");

    KeySign keySign = NoteTableGetKeySign(keyChar, sharp, flat, major);
    if (KeySignInvalid == keySign) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    KeySignExpr *self = ExpressionCreate(location, sizeof(KeySignExpr), "key sign");
    self->expr.vtbl.parse = KeySignExprParse;
    self->keySign = keySign;

    free(keyString);

    return self;
}

typedef struct _RestExpr {
    Expression expr;
    int step;
} RestExpr;

static bool RestExprParse(void *_self, void *parser, void *_context)
{
    RestExpr *self = _self;
    NAMidiParserContext *context = _context;
    context->channels[context->channel].tick += self->step;
    return true;
}

void *NAMidiExprRest(NAMidiParser *parser, ParseLocation *location, int step)
{ __Trace__
    if (!isValidRange(step, 0, 65535)) {
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
    }

    RestExpr *self = ExpressionCreate(location, sizeof(RestExpr), "rest");
    self->expr.vtbl.parse = RestExprParse;
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

static bool NoteExprParse(void *_self, void *parser, void *_context)
{
    NoteExpr *self = _self;
    NAMidiParserContext *context = _context;
    SequenceBuilder *builder = NAMidiParserGetBuilder(parser);

    int octave = OCTAVE_NONE != self->octave ? self->octave : context->channels[context->channel].octave;
    context->channels[context->channel].octave = octave;

    int noteNo = NoteTableGetNoteNo(context->keySign, self->baseNote, self->accidental, octave);
    if (!isValidRange(noteNo, 0, 127)) {
        NAMidiParserError(parser, &self->expr.location, ParseErrorKindGeneral, GeneralParseErrorInvalidNoteRange);
        return false;
    }

    int step = -1 != self->step ? self->step : 0;

    int gatetime = -1 != self->gatetime ? self->gatetime : context->channels[context->channel].gatetime;
    context->channels[context->channel].gatetime = gatetime;

    int velocity = -1 != self->velocity ? self->velocity : context->channels[context->channel].velocity;
    context->channels[context->channel].velocity = velocity;

    builder->appendNote(builder, ++context->id, context->channels[context->channel].tick, context->channel, noteNo, gatetime, velocity);
    context->channels[context->channel].tick += step;

    return true;
}

void *NAMidiExprNote(NAMidiParser *parser, ParseLocation *location, char *noteString, int step, int gatetime, int velocity)
{ __Trace__
    const BaseNote noteTable[] = {
        BaseNote_A, BaseNote_B, BaseNote_C,
        BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
    };

    char *pc = noteString;

    BaseNote baseNote = noteTable[tolower(*pc) - 97];
    Accidental accidental = AccidentalNone;

    int octave = OCTAVE_NONE;
    char *c;
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
        NAMidiParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    NoteExpr *self = ExpressionCreate(location, sizeof(NoteExpr), "note");
    self->expr.vtbl.parse = NoteExprParse;

    self->step = step;
    self->baseNote = baseNote;
    self->accidental = accidental;
    self->octave = octave;
    self->gatetime = gatetime;
    self->velocity = velocity;

    free(noteString);

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

void *NAMidiExprPattern(NAMidiParser *parser, ParseLocation *location, char *identifier, Expression *statementList)
{ __Trace__
    PatternExpr *self = ExpressionCreate(location, sizeof(PatternExpr), PATTERN_ID);
    self->expr.vtbl.destroy = PatternExprDestroy;
    self->identifier = identifier;
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

    if (self->contextIdList) {
        NAArrayTraverse(self->contextIdList, free);
        NAArrayDestroy(self->contextIdList);
    }

    free(self);
}

static bool PatternExpandExprParse(void *_self, void *parser, void *_context)
{
    PatternExpandExpr *self = _self;
    NAMidiParserContext *context = _context;
    Expression *pattern = NAMapGet(context->patternMap, self->identifier);
    if (!pattern) {
        NAMidiParserError(parser, &self->expr.location, ParseErrorKindNAMidi, NAMidiParseErrorPatternMissing);
        return false;
    }

    if (self->contextIdList) {
        int count = NAArrayCount(self->contextIdList);
        void **values = NAArrayGetValues(self->contextIdList);
        for (int i = 0; i < count; ++i) {
            NASetAdd(context->contextIdList, values[i]);
        }
    }

    Expression *statementList = NAArrayGetValueAt(pattern->children, 0);
    return ExpressionParse(statementList, parser, context);
}

void *NAMidiExprPatternExpand(NAMidiParser *parser, ParseLocation *location, char *identifier, NAArray *idList)
{ __Trace__
    PatternExpandExpr *self = ExpressionCreate(location, sizeof(PatternExpandExpr), "pattern expand");
    self->expr.vtbl.destroy = PatternExprExpandDestroy;
    self->expr.vtbl.parse = PatternExpandExprParse;
    self->identifier = identifier;
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

void *NAMidiExprContext(NAMidiParser *parser, ParseLocation *location, NAArray *idList, Expression *statementList)
{ __Trace__
    ContextExpr *self = ExpressionCreate(location, sizeof(ContextExpr), "context");
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

NAMap *NAMidiExprStatementListGetPatternMap(void *_self)
{
    StatementListExpr *self = _self;
    return self->patternMap;
}

char *NAMidiExprPatternGetIdentifier(void *_self)
{
    PatternExpr *self = _self;
    return self->identifier;
}

Expression *NAMidiExprStatementListMarge(Expression *self, Expression *statementList)
{
    int count = NAArrayCount(statementList->children);
    void **values = NAArrayGetValues(statementList->children);
    for (int i = 0; i < count; ++i) {
        Expression *child = values[i];
        child->parent = self;
        NAArrayAppend(self->children, child);
    }

    NAArrayRemoveAll(statementList->children);
    ExpressionDestroy(statementList);
    return self;
}
