#include "ABCExpression.h"
#include "NoteTable.h"
#include "NAUtil.h"

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

    NAIterator *iterator = NAArrayGetIterator(self->expr.children);
    while (iterator->hasNext(iterator)) {
        if (!ExpressionParse(iterator->next(iterator), parser, context)) {
            success = false;
            goto ERROR;
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
    StatementListExpr *self = ExpressionCreate(location, sizeof(StatementListExpr), STATEMENT_LIST_ID);
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
        ABCParserError(parser, &self->expr.location, ParseErrorKindABC, ABCParseErrorUnexpectedVersionExpression);
        return false;
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
            ABCParserError(parser, &self->expr.location, ParseErrorKindABC, ABCParseErrorUnrecognisedVersion);
            return false;
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

typedef struct _KeyExpr {
    Expression expr;
    KeySign keySign;
} KeyExpr;

static bool KeyExprParse(void *_self, void *parser, void *_context)
{
    KeyExpr *self = _self;
    ABCParserContext *context = _context;
    context->keySign = self->keySign;

    uint8_t sf, mi;
    KeySignGetMidiExpression(self->keySign, &sf, &mi);

    SequenceBuilder *builder = ABCParserGetBuilder(parser);
    builder->appendKey(builder, context->channels[context->channel].tick, sf, mi);
    return true;
}

void *ABCExprKey(ABCParser *parser, ParseLocation *location, char *keyName, char *keyScale)
{ __Trace__
    NAUtilToLowerCase(keyName);

    char keyChar = keyName[0];
    bool sharp = NULL != strchr(&keyName[1], '#');
    bool flat = NULL != strchr(&keyName[1], 'b');
    bool major = true;

    free(keyName);
    
    if (keyScale) {
        NAUtilToLowerCase(keyScale);

        const struct {
            const char *name;
            bool major;
        } scales[] = {
            {"ma", true}, {"maj", true}, {"major", true},
            {"m", false}, {"min", false}, {"minor", false},
            {"ion", true}, {"ionian", true},
            {"aeo", false}, {"aeolian", false},
        };

        for (int i = 0; i < sizeof(scales)/sizeof(scales[0]); ++i) {
            if (0 == strcmp(keyScale, scales[i].name)) {
                major = scales[i].major;
                goto KEY_FOUND;
            }
        }

        const struct {
            const char *name;
            const char *abbr;
            struct {
                struct {
                    char keyChar;
                    bool sharp;
                    bool flat;
                } normal;
                struct {
                    char keyChar;
                    bool sharp;
                    bool flat;
                } mode;
            } tbl[15];
        } modes[] = {
            {
                "mixolydian", "mix",
                {
                    {{'c', true, false}, {'g', true, false}},
                    {{'f', true, false}, {'c', true, false}},
                    {{'b', false, false}, {'f', true, false}},
                    {{'e', false, false}, {'b', false, false}},
                    {{'a', false, false}, {'e', false, false}},
                    {{'d', false, false}, {'a', false, false}},
                    {{'g', false, false}, {'d', false, false}},
                    {{'c', false, false}, {'g', false, false}},
                    {{'f', false, false}, {'c', false, false}},
                    {{'b', false, true}, {'f', false, false}},
                    {{'e', false, true}, {'b', false, true}},
                    {{'a', false, true}, {'e', false, true}},
                    {{'d', false, true}, {'a', false, true}},
                    {{'g', false, true}, {'d', false, true}},
                    {{'c', false, true}, {'g', false, true}},
                }
            },
            {
                "dorian", "dor",
                {
                    {{'c', true, false}, {'d', true, false}},
                    {{'f', true, false}, {'g', true, false}},
                    {{'b', false, false}, {'c', true, false}},
                    {{'e', false, false}, {'f', true, false}},
                    {{'a', false, false}, {'b', false, false}},
                    {{'d', false, false}, {'e', false, false}},
                    {{'g', false, false}, {'a', false, false}},
                    {{'c', false, false}, {'d', false, false}},
                    {{'f', false, false}, {'g', false, false}},
                    {{'b', false, true}, {'c', false, false}},
                    {{'e', false, true}, {'f', false, false}},
                    {{'a', false, true}, {'b', false, true}},
                    {{'d', false, true}, {'e', false, true}},
                    {{'g', false, true}, {'a', false, true}},
                    {{'c', false, true}, {'d', false, true}},
                }
            },
            {
                "phrygian", "phr",
                {
                    {{'c', true, false}, {'e', true, false}},
                    {{'f', true, false}, {'a', true, false}},
                    {{'b', false, false}, {'d', true, false}},
                    {{'e', false, false}, {'g', true, false}},
                    {{'a', false, false}, {'c', true, false}},
                    {{'d', false, false}, {'f', true, false}},
                    {{'g', false, false}, {'b', false, false}},
                    {{'c', false, false}, {'e', false, false}},
                    {{'f', false, false}, {'a', false, false}},
                    {{'b', false, true}, {'d', false, false}},
                    {{'e', false, true}, {'g', false, false}},
                    {{'a', false, true}, {'c', false, false}},
                    {{'d', false, true}, {'f', false, false}},
                    {{'g', false, true}, {'b', false, true}},
                    {{'c', false, true}, {'e', false, true}},
                }
            },
            {
                "lydian", "lyd",
                {
                    {{'c', true, false}, {'f', true, false}},
                    {{'f', true, false}, {'b', false, false}},
                    {{'b', false, false}, {'e', false, false}},
                    {{'e', false, false}, {'a', false, false}},
                    {{'a', false, false}, {'d', false, false}},
                    {{'d', false, false}, {'g', false, false}},
                    {{'g', false, false}, {'c', false, false}},
                    {{'c', false, false}, {'f', false, false}},
                    {{'f', false, false}, {'b', false, true}},
                    {{'b', false, true}, {'e', false, true}},
                    {{'e', false, true}, {'a', false, true}},
                    {{'a', false, true}, {'d', false, true}},
                    {{'d', false, true}, {'g', false, true}},
                    {{'g', false, true}, {'c', false, true}},
                    {{'c', false, true}, {'f', false, true}},
                }
            },
            {
                "locrian", "loc",
                {
                    {{'c', true, false}, {'b', true, false}},
                    {{'f', true, false}, {'e', true, false}},
                    {{'b', false, false}, {'a', true, false}},
                    {{'e', false, false}, {'d', true, false}},
                    {{'a', false, false}, {'g', true, false}},
                    {{'d', false, false}, {'c', true, false}},
                    {{'g', false, false}, {'f', true, false}},
                    {{'c', false, false}, {'b', false, false}},
                    {{'f', false, false}, {'e', false, false}},
                    {{'b', false, true}, {'a', false, false}},
                    {{'e', false, true}, {'d', false, false}},
                    {{'a', false, true}, {'g', false, false}},
                    {{'d', false, true}, {'c', false, false}},
                    {{'g', false, true}, {'f', false, false}},
                    {{'c', false, true}, {'b', false, true}},
                }
            }
        };

        for (int i = 0; i < sizeof(modes)/sizeof(modes[0]); ++i) {
            if (0 == strcmp(keyScale, modes[i].name)
                    || 0 == strcmp(keyScale, modes[i].abbr)) {
                for (int j = 0; j < 15; ++j) {
                    if (modes[i].tbl[j].mode.keyChar == keyChar
                            && modes[i].tbl[j].mode.sharp == sharp
                            && modes[i].tbl[j].mode.flat == flat) {
                        keyChar = modes[i].tbl[j].normal.keyChar;
                        sharp = modes[i].tbl[j].normal.sharp;
                        flat = modes[i].tbl[j].normal.flat;
                        goto KEY_FOUND;
                    }
                }
            }
        }

        ABCParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        free(keyScale);
        return NULL;
    }
KEY_FOUND:

    free(keyScale);

    KeySign keySign = NoteTableGetKeySign(keyChar, sharp, flat, major);
    if (KeySignInvalid == keySign) {
        ABCParserError(parser, location, ParseErrorKindGeneral, GeneralParseErrorInvalidValue);
        return NULL;
    }

    KeyExpr *self = ExpressionCreate(location, sizeof(KeyExpr), "key");
    self->expr.vtbl.parse = KeyExprParse;
    self->keySign = keySign;

    return self;
}

typedef struct _NoteExpr {
    Expression expr;
    int step;
    BaseNote baseNote;
    Accidental accidental;
    int octave;
    int gatetime;
} NoteExpr;

static bool NoteExprParse(void *_self, void *parser, void *_context)
{
    NoteExpr *self = _self;
    ABCParserContext *context = _context;
    SequenceBuilder *builder = ABCParserGetBuilder(parser);

    int noteNo = NoteTableGetNoteNo(context->keySign, self->baseNote, self->accidental, self->octave)
        + context->transpose;
    if (!isValidRange(noteNo, 0, 127)) {
        ABCParserError(parser, &self->expr.location, ParseErrorKindGeneral, GeneralParseErrorInvalidNoteRange);
        return false;
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
    NoteExpr *self = NULL;

    const BaseNote noteTable[] = {
        BaseNote_A, BaseNote_B, BaseNote_C,
        BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
    };

    char *pc = noteString;

    BaseNote baseNote = noteTable[tolower(*pc) - 97];
    Accidental accidental = AccidentalNone;
    int octave = isupper(*pc) ? 2 : 3;

    char *c;
    while (*(c = ++pc)) {
        switch (*c) {
        case ',':
            if (3 == octave) {
                ABCParserError(parser, location, ParseErrorKindABC, ABCParseErrorIllegalOctaveDown);
                goto ERROR;
            }
            --octave;
            break;
        case '\'':
            if (2 == octave) {
                ABCParserError(parser, location, ParseErrorKindABC, ABCParseErrorIllegalOctaveUp);
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

    self = ExpressionCreate(location, sizeof(NoteExpr), "note");
    self->expr.vtbl.parse = NoteExprParse;

    self->step = step;
    self->baseNote = baseNote;
    self->accidental = accidental;
    self->gatetime = gatetime;
    self->octave = octave;

ERROR:
    free(noteString);

    return self;
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
