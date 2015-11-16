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
    StatementListExpr *self = _self;
    ABCParserContext *context = _context;

    // TODO
    printf("EOL\n");
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
{
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
