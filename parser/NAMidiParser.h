#pragma once

#include "Parser.h"

typedef enum {
    NAMidiParseErrorPatternMissing,
} NAMidiParseError;

extern Parser *NAMidiParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver);

static inline const char *NAMidiParseError2String(NAMidiParseError error)
{
#define CASE(error) case error: return &(#error[11])
    switch (error) {
    CASE(NAMidiParseErrorPatternMissing);
    default:
       break;
    }

    return "Unknown error";
#undef CASE
}

#include "Expression.h"
#include "NoteTable.h"
#include "SequenceBuilder.h"
#include "NAMap.h"
#include "NASet.h"

typedef struct _NAMidiParser NAMidiParser;
extern bool NAMidiParserReadIncludeFile(NAMidiParser *self, const char *filepath, int line, int column, char *includeFile, Expression **expression);
extern void NAMidiParserError(NAMidiParser *self, const char *filepath, int line, int column, ParseErrorKind kind, int error);
extern SequenceBuilder *NAMidiParserGetBuilder(NAMidiParser *self);

typedef struct _NAMidiParserContext {
    int id;
    int channel;
    int transpose;
    KeySign keySign;
    struct {
        int tick;
        int gatetime;
        int velocity;
        int octave;
    } channels[16];

    NASet *contextIdList;

    NAMap *patternMap;
    NASet *expandingPatternList;

    bool copy;
} NAMidiParserContext;

extern NAMidiParserContext *NAMidiParserContextCreate();
extern NAMidiParserContext *NAMidiParserContextCreateCopy(NAMidiParserContext *self);
extern void NAMidiParserContextDestroy(NAMidiParserContext *self);
