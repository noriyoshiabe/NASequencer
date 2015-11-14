#pragma once

#include "Parser.h"

typedef enum {
    ABCParseErrorUnsupportedVersion,
    ABCParseErrorIllegalOctaveDown,
    ABCParseErrorIllegalOctaveUp,
} ABCParseError;

extern Parser *ABCParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver);

static inline const char *ABCParseError2String(ABCParseError error)
{
#define CASE(error) case error: return &(#error[13])
    switch (error) {
    CASE(ABCParseErrorUnsupportedVersion);
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
extern void ABCParserError(ABCParser *self, ParseLocation *location, ParseErrorKind kind, int error);
extern SequenceBuilder *ABCParserGetBuilder(ABCParser *self);

typedef struct _ABCParserContext {
    int id;
    int channel;
    int transpose;
    KeySign keySign;
    struct {
        int tick;
    } channels[16];
} ABCParserContext;

extern ABCParserContext *ABCParserContextCreate();
extern void ABCParserContextDestroy(ABCParserContext *self);
extern int ABCParserContextGetLength(ABCParserContext *self);
