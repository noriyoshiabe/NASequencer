#pragma once

#include "Parser.h"

typedef enum {
    ABCParseErrorUnrecognisedVersion = ParseErrorKindABC,
    ABCParseErrorUnexpectedVersionExpression,
    ABCParseErrorInvalidKeyMode,
    ABCParseErrorInvalidKeySign,
    ABCParseErrorInvalidNoteNumber,
    ABCParseErrorIllegalOctaveDown,
    ABCParseErrorIllegalOctaveUp,
} ABCParseError;

extern DSLParser *ABCParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver);

static inline const char *ABCParseError2String(ABCParseError error)
{
#define CASE(error) case error: return &(#error[13])
    switch (error) {
    CASE(ABCParseErrorUnrecognisedVersion);
    CASE(ABCParseErrorUnexpectedVersionExpression);
    CASE(ABCParseErrorInvalidKeyMode);
    CASE(ABCParseErrorInvalidKeySign);
    CASE(ABCParseErrorInvalidNoteNumber);
    CASE(ABCParseErrorIllegalOctaveDown);
    CASE(ABCParseErrorIllegalOctaveUp);
    default:
       break;
    }

    return "Unknown error";
#undef CASE
}

#include "Expression.h"
#include "NoteTable.h"
#include "SequenceBuilder.h"

typedef struct _ABCParser ABCParser;
extern void ABCParserError(ABCParser *self, ParseLocation *location, int code, ...);
extern SequenceBuilder *ABCParserGetBuilder(ABCParser *self);

typedef struct _ABCParserContext {
    int channel;
    int transpose;
    NoteTable *noteTable;
    struct {
        int tick;
    } channels[16];

    bool strict;
    struct {
        int major;
        int minor;
        const char *text;
    } version;
} ABCParserContext;

extern ABCParserContext *ABCParserContextCreate();
extern void ABCParserContextDestroy(ABCParserContext *self);
extern int ABCParserContextGetLength(ABCParserContext *self);
