#include "ABCExpression.h"
#include "NoteTable.h"
#include "NAUtil.h"
#include "NACString.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define isValidRange(v, from, to) (from <= v && v <= to)

#if 0
#define __Trace__ printf("%s - %s:%d:%d\n", __func__, filepath, ((int *)yylloc)[0], ((int *)yylloc)[1]);
#else
#define __Trace__
#endif

static const char *STATEMENT_LIST_ID = "statement list";
static const char *KEY_EXPR_LIST_ID = "key expr list";

static BaseNote KeyChar2BaseNote(char c)
{
    const BaseNote baseNoteTable[] = {
        BaseNote_A, BaseNote_B, BaseNote_C,
        BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
    };

    return baseNoteTable[tolower(c) - 97];
}

typedef struct _StatementListExpr {
    Expression expr;
} StatementListExpr;

static void StatementListExprDestroy(void *_self)
{
    StatementListExpr *self = _self;
    free(self);
}

static bool StatementListExprParse(void *_self, void *parser, void *_context)
{
    StatementListExpr *self = _self;
    ABCParserContext *parent = _context;

    ABCParserContext *context = parent ? parent : ABCParserContextCreate();

    bool success = true;

    if (self->expr.children) {
        NAIterator *iterator = NAArrayGetIterator(self->expr.children);
        while (iterator->hasNext(iterator)) {
            if (!ExpressionParse(iterator->next(iterator), parser, context)) {
                success = false;
                goto ERROR;
            }
        }
    }

    if (!parent) {
        SequenceBuilder *builder = ABCParserGetBuilder(parser);
        builder->setLength(builder, ABCParserContextGetLength(context));
    }

ERROR:
    if (!parent) {
        ABCParserContextDestroy(context);
    }

    return success;
}

void *ABCExprStatementList(ABCParser *parser, ParseLocation *location)
{ __Trace__
    StatementListExpr *self = ExpressionCreateListExpr(location, sizeof(StatementListExpr), STATEMENT_LIST_ID);
    self->expr.vtbl.destroy = StatementListExprDestroy;
    self->expr.vtbl.parse = StatementListExprParse;
    return self;
}

typedef struct _EOLExpr {
    Expression expr;
} EOLExpr;

static void EOLExprDestroy(void *self)
{
    free(self);
}

static bool EOLExprParse(void *_self, void *parser, void *_context)
{
    // TODO
    return true;
}

void *ABCExprEOL(ABCParser *parser, ParseLocation *location)
{ __Trace__
    EOLExpr *self = ExpressionCreate(location, sizeof(EOLExpr), "eol");
    self->expr.vtbl.destroy = EOLExprDestroy;
    self->expr.vtbl.parse = EOLExprParse;
    return self;
}

typedef struct _VersionExpr {
    Expression expr;
    char *version;
} VersionExpr;

static void VersionExprDestroy(void *_self)
{
    VersionExpr *self = _self;
    free(self->version);
    free(self);
}

static bool VersionExprParse(void *_self, void *parser, void *_context)
{
    VersionExpr *self = _self;
    ABCParserContext *context = _context;
    if (context->version.text) {
        ABCParserError(parser, &self->expr.location, ABCParseErrorUnexpectedVersionExpression, context->version.text, NULL);
        return true;
    }

    context->version.text = self->version;

    char *buffer = alloca(strlen(self->version) + 1);
    strcpy(buffer, self->version);

    char *saveptr, *token, *s = buffer;
    for (int i = 0; (token = strtok_r(s, ".", &saveptr)); ++i) {
        int *ver = 0 == i ? &context->version.major : &context->version.minor;
        char *err;
        *ver = strtol(token, &err, 10);
        if ('\0' != *err) {
            ABCParserError(parser, &self->expr.location, ABCParseErrorUnrecognisedVersion, self->version, NULL);
            return true;
        }
        s = NULL;
    }

    context->strict = 2 <= context->version.major && 1 <= context->version.minor;
    return true;
}

void *ABCExprVersion(ABCParser *parser, ParseLocation *location, char *version)
{ __Trace__
    VersionExpr *self = ExpressionCreate(location, sizeof(VersionExpr), "version");
    self->expr.vtbl.destroy = VersionExprDestroy;
    self->expr.vtbl.parse = VersionExprParse;
    self->version = version;
    return self;
}

void *ABCExprReferenceNumber(ABCParser *parser, ParseLocation *location, char *numberString)
{ __Trace__
    // Unsupported
    free(numberString);

    return ExpressionCreate(location, sizeof(Expression), "reference number");
}

typedef struct _TuneTitleExpr {
    Expression expr;
    char *title;
} TuneTitleExpr;

static void TuneTitleExprDestroy(void *_self)
{
    TuneTitleExpr *self = _self;
    free(self->title);
    free(self);
}

static bool TuneTitleExprParse(void *_self, void *parser, void *_context)
{
    TuneTitleExpr *self = _self;

    if (0 < strlen(self->title)) {
        SequenceBuilder *builder = ABCParserGetBuilder(parser);
        builder->setTitle(builder, self->title);
    }

    return true;
}

void *ABCExprTuneTitle(ABCParser *parser, ParseLocation *location, char *title)
{ __Trace__
    TuneTitleExpr *self = ExpressionCreate(location, sizeof(TuneTitleExpr), "tune title");
    self->expr.vtbl.destroy = TuneTitleExprDestroy;
    self->expr.vtbl.parse = TuneTitleExprParse;
    self->title = NAUtilTrimWhiteSpace(title);
    return self;
}

static bool KeyExprListExprParse(void *_self, void *parser, void *context)
{
    Expression *self = _self;
    NAIterator *iterator = NAArrayGetIterator(self->children);
    while (iterator->hasNext(iterator)) {
        if (!ExpressionParse(iterator->next(iterator), parser, context)) {
            return false;
        }
    }

    return true;
}

void *ABCExprKeyExprList(ABCParser *parser, ParseLocation *location)
{ __Trace__
    Expression *self = ExpressionCreate(location, sizeof(Expression), KEY_EXPR_LIST_ID);
    self->vtbl.parse = KeyExprListExprParse;
    return self;
}

typedef struct _KeySignExpr {
    Expression expr;
    NoteTable *noteTable;
} KeySignExpr;

static void KeySignExprDestroy(void *_self)
{
    KeySignExpr *self = _self;
    NoteTableRelease(self->noteTable);
    free(self);
}

static bool KeySignExprParse(void *_self, void *parser, void *_context)
{
    KeySignExpr *self = _self;
    ABCParserContext *context = _context;

    NoteTableRelease(context->noteTable);
    context->noteTable = NoteTableRetain(self->noteTable);

    MidiKeySign keysign = NoteTableGetMidiKeySign(self->noteTable);

    SequenceBuilder *builder = ABCParserGetBuilder(parser);
    builder->appendKey(builder, context->channels[context->channel].tick, keysign.sf, keysign.mi);
    return true;
}

void *ABCExprKeySign(ABCParser *parser, ParseLocation *location, char *tonicString, char *modeString, NAArray *accidentals)
{ __Trace__
    KeySignExpr *self = NULL;

    BaseNote baseNote = BaseNote_C;
    bool sharp = false;
    bool flat = false;
    Mode mode = ModeMajor;

    if (tonicString) {
        char *_tonicString = NACStringDuplicate(tonicString);
        NAUtilToLowerCase(_tonicString);

        if (0 == strcmp("none", _tonicString)) {
            goto KEY_FOUND;
        }

        if (0 == strcmp("hp", _tonicString)) {
            baseNote = BaseNote_D;
            goto KEY_FOUND;
        }

        baseNote = KeyChar2BaseNote(_tonicString[0]);
        sharp = NULL != strchr(&_tonicString[1], '#');
        flat = NULL != strchr(&_tonicString[1], 'b');
    }
    
    if (modeString) {
        char *_modeString = NACStringDuplicate(modeString);
        NAUtilToLowerCase(_modeString);

        const struct {
            const char *name;
            Mode mode;
        } scales[] = {
            {"ma", ModeMajor}, {"maj", ModeMajor}, {"major", ModeMajor},
            {"m", ModeMinor}, {"min", ModeMinor}, {"minor", ModeMinor},
            {"ion", ModeIonian}, {"ionian", ModeIonian},
            {"aeo", ModeAeolian}, {"aeolian", ModeAeolian},
            {"mix", ModeMixolydian}, {"mixolydian", ModeMixolydian},
            {"dor", ModeDorian}, {"dorian", ModeDorian},
            {"phr", ModePhrygian}, {"phrygian", ModePhrygian},
            {"lyd", ModeLydian}, {"lydian", ModeLydian},
            {"loc", ModeLocrian}, {"locrian", ModeLocrian},
            {"exp", ModeMajor}, {"explicit", ModeMajor},
        };

        for (int i = 0; i < sizeof(scales)/sizeof(scales[0]); ++i) {
            if (0 == strcmp(_modeString, scales[i].name)) {
                mode = scales[i].mode;
                goto KEY_FOUND;
            }
        }

        ABCParserError(parser, location, ABCParseErrorInvalidKeyMode, modeString, NULL);
        goto EXIT;
    }

KEY_FOUND:
    ;

    NoteTable *noteTable = NoteTableCreate(baseNote, sharp, flat, mode);
    if (NoteTableHasUnusualKeySign(noteTable)) {
        ABCParserError(parser, location, ABCParseErrorInvalidKeySign, tonicString, modeString, NULL);
        NoteTableRelease(noteTable);
        goto EXIT;
    }

    if (accidentals) {
        NAIterator *iterator = NAArrayGetIterator(accidentals);
        while (iterator->hasNext(iterator)) {
            char *accidentalString = iterator->next(iterator);
            char *pc = accidentalString;
            char c;

            BaseNote baseNote = BaseNote_C;
            Accidental accidental = AccidentalNone;

            while ((c = *(pc++))) {
                switch (c) {
                case '^':
                    accidental = AccidentalSharp == accidental ? AccidentalDoubleSharp : AccidentalSharp;
                    break;
                case '_':
                    accidental = AccidentalFlat == accidental ? AccidentalDoubleFlat : AccidentalFlat;
                    break;
                case '=':
                    accidental = AccidentalNatural;
                    break;
                default:
                    baseNote = KeyChar2BaseNote(tolower(c));
                    break;
                }
            }

            NoteTableAppendAccidental(noteTable, baseNote, accidental);
        }
    }

    self = ExpressionCreate(location, sizeof(KeySignExpr), "key sign");
    self->expr.vtbl.destroy = KeySignExprDestroy;
    self->expr.vtbl.parse = KeySignExprParse;
    self->noteTable = noteTable;

EXIT:
    if (tonicString) {
        free(tonicString);
    }

    if (modeString) {
        free(modeString);
    }

    if (accidentals) {
        NAArrayTraverse(accidentals, free);
        NAArrayDestroy(accidentals);
    }

    return self;
}

typedef struct _NoteExpr {
    Expression expr;
    int step;
    BaseNote baseNote;
    Accidental accidental;
    int octave;
    int gatetime;
    char *noteString;
} NoteExpr;

static void NoteExprDestroy(void *_self)
{
    NoteExpr *self = _self;
    free(self->noteString);
    free(self);
}

static bool NoteExprParse(void *_self, void *parser, void *_context)
{
    NoteExpr *self = _self;
    ABCParserContext *context = _context;
    SequenceBuilder *builder = ABCParserGetBuilder(parser);

    int noteNo = NoteTableGetNoteNo(context->noteTable, self->baseNote, self->accidental, self->octave)
        + context->transpose;
    if (!isValidRange(noteNo, 0, 127)) {
        ABCParserError(parser, &self->expr.location, ParseErrorKindABC, ABCParseErrorInvalidNoteNumber,
                NACStringFromInteger(noteNo), self->noteString, NULL);
        return true;
    }

    int step = self->step; // TODO chord
    int gatetime = self->gatetime;
    int velocity = 127; // TODO from context

    builder->appendNote(builder, context->channels[context->channel].tick, context->channel, noteNo, gatetime, velocity);
    context->channels[context->channel].tick += step;

    return true;
}

void *ABCExprNote(ABCParser *parser, ParseLocation *location, char *noteString)
{ __Trace__
    char *pc = noteString;

    BaseNote baseNote = KeyChar2BaseNote(*pc);
    Accidental accidental = AccidentalNone;
    int octave = isupper(*pc) ? 2 : 3;

    char *c;
    while (*(c = ++pc)) {
        switch (*c) {
        case ',':
            if (3 == octave) {
                ABCParserError(parser, location, ABCParseErrorIllegalOctaveDown, noteString, NULL);
                goto ERROR;
            }
            --octave;
            break;
        case '\'':
            if (2 == octave) {
                ABCParserError(parser, location, ABCParseErrorIllegalOctaveUp, noteString, NULL);
                goto ERROR;
            }
            ++octave;
            break;
        case '^':
            accidental = AccidentalSharp == accidental ? AccidentalDoubleSharp : AccidentalSharp;
            break;
        case '_':
            accidental = AccidentalFlat == accidental ? AccidentalDoubleFlat : AccidentalFlat;
            break;
        case '=':
            accidental = AccidentalNatural;
            break;
        }
    }

    int step = 240; // TODO length
    int gatetime = 240; // TODO length

    NoteExpr *self = ExpressionCreate(location, sizeof(NoteExpr), "note");
    self->expr.vtbl.destroy = NoteExprDestroy;
    self->expr.vtbl.parse = NoteExprParse;

    self->step = step;
    self->baseNote = baseNote;
    self->accidental = accidental;
    self->gatetime = gatetime;
    self->octave = octave;

    self->noteString = noteString;

    return self;

ERROR:
    free(noteString);

    return NULL;
}

bool ABCExprIsStatementList(Expression *self)
{
    return self->identifier == STATEMENT_LIST_ID;
}

Expression *ABCExprStatementListMarge(Expression *self, Expression *statementList)
{
    NAIterator *iterator = NAArrayGetIterator(statementList->children);
    while (iterator->hasNext(iterator)) {
        Expression *child = iterator->next(iterator);
        child->parent = self;
        NAArrayAppend(self->children, child);
    }

    NAArrayRemoveAll(statementList->children);
    ExpressionDestroy(statementList);
    return self;
}

bool ABCExprIsKeyExprList(Expression *self)
{
    return self->identifier == KEY_EXPR_LIST_ID;
}

Expression *ABCExprKeyExprListMarge(Expression *self, Expression *statementList)
{
    NAIterator *iterator = NAArrayGetIterator(statementList->children);
    while (iterator->hasNext(iterator)) {
        Expression *child = iterator->next(iterator);
        child->parent = self;
        NAArrayAppend(self->children, child);
    }

    NAArrayRemoveAll(statementList->children);
    ExpressionDestroy(statementList);
    return self;
}
