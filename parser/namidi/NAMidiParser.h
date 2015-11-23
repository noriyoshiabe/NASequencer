#pragma once

#include "Parser.h"

typedef enum {
    NAMidiParseErrorInvalidResolution = ParseErrorKindNAMidi,
    NAMidiParseErrorInvalidTempo,
    NAMidiParseErrorInvalidTimeSign,
    NAMidiParseErrorInvalidChannel,
    NAMidiParseErrorInvalidVoice,
    NAMidiParseErrorInvalidVolume,
    NAMidiParseErrorInvalidPan,
    NAMidiParseErrorInvalidChorus,
    NAMidiParseErrorInvalidReverb,
    NAMidiParseErrorInvalidTranspose,
    NAMidiParseErrorInvalidKeySign,
    NAMidiParseErrorInvalidStep,
    NAMidiParseErrorInvalidNoteNumber,
    NAMidiParseErrorInvalidOctave,
    NAMidiParseErrorInvalidGatetime,
    NAMidiParseErrorInvalidVelocity,
    NAMidiParseErrorUnsupportedFileTypeInclude,
    NAMidiParseErrorIncludeFileNotFound,
    NAMidiParseErrorCircularFileInclude,
    NAMidiParseErrorPatternMissing,
    NAMidiParseErrorDuplicatePatternIdentifier,
    NAMidiParseErrorCircularPatternReference,
} NAMidiParseError;

extern DSLParser *NAMidiParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver);

static inline const char *NAMidiParseError2String(NAMidiParseError error)
{
#define CASE(error) case error: return &(#error[16])
    switch (error) {
    CASE(NAMidiParseErrorInvalidResolution);
    CASE(NAMidiParseErrorInvalidTempo);
    CASE(NAMidiParseErrorInvalidTimeSign);
    CASE(NAMidiParseErrorInvalidChannel);
    CASE(NAMidiParseErrorInvalidVoice);
    CASE(NAMidiParseErrorInvalidVolume);
    CASE(NAMidiParseErrorInvalidPan);
    CASE(NAMidiParseErrorInvalidChorus);
    CASE(NAMidiParseErrorInvalidReverb);
    CASE(NAMidiParseErrorInvalidTranspose);
    CASE(NAMidiParseErrorInvalidKeySign);
    CASE(NAMidiParseErrorInvalidStep);
    CASE(NAMidiParseErrorInvalidNoteNumber);
    CASE(NAMidiParseErrorInvalidOctave);
    CASE(NAMidiParseErrorInvalidGatetime);
    CASE(NAMidiParseErrorInvalidVelocity);
    CASE(NAMidiParseErrorUnsupportedFileTypeInclude);
    CASE(NAMidiParseErrorIncludeFileNotFound);
    CASE(NAMidiParseErrorCircularFileInclude);
    CASE(NAMidiParseErrorPatternMissing);
    CASE(NAMidiParseErrorDuplicatePatternIdentifier);
    CASE(NAMidiParseErrorCircularPatternReference);
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
extern bool NAMidiParserReadIncludeFile(NAMidiParser *self, ParseLocation *location, char *includeFile, Expression **expression);
extern void NAMidiParserError(NAMidiParser *self, ParseLocation *location, int code, ...);
extern SequenceBuilder *NAMidiParserGetBuilder(NAMidiParser *self);

typedef struct _NAMidiParserContext {
    int channel;
    int transpose;
    NoteTable *noteTable;
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
extern int NAMidiParserContextGetLength(NAMidiParserContext *self);
