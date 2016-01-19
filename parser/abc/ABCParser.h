#pragma once

#include "DSLParser.h"
#include "ParseInfo.h"

typedef enum {
    ABCParseErrorUnrecognisedVersion = ParseErrorKindABC,
    ABCParseErrorUnsupportedFileTypeInclude,
    ABCParseErrorCircularFileInclude,
    ABCParseErrorIncludeFileNotFound,

    ABCParseErrorIllegalStateWithTitle,
    ABCParseErrorIllegalStateWithKey,
    ABCParseErrorDuplicatedKeyTonic,
    ABCParseErrorDuplicatedKeyMode,
    ABCParseErrorInvalidTranspose,
    ABCParseErrorInvalidOctave,
    ABCParseErrorInvalidKeyMode,
    ABCParseErrorInvalidKey,
    ABCParseErrorInvalidMeter,
    ABCParseErrorInvalidUnitNoteLength,
    ABCParseErrorInvalidBeatUnit,
    ABCParseErrorInvalidBeatCount,
    ABCParseErrorInvalidTempo,
    ABCParseErrorIllegalStateWithParts,
    ABCParseErrorDuplicatedParts,
    ABCParseErrorIllegalStateWithCopyright,
    ABCParseErrorIllegalStateWithInclude,
    ABCParseErrorIllegalStateWithVoice,
    ABCParseErrorIllegalStateWithTuneBody,
    ABCParseErrorInvalidNoteLength,
    ABCParseErrorInvalidRepeat,
    ABCParseErrorInvalidNthRepeat,
    ABCParseErrorIllegalStateWithMidiVoice,
    ABCParseErrorDuplicatedMidiVoiceId,
    ABCParseErrorInvalidMidiInstrument,
    ABCParseErrorMidiVoiceIdMissingInTune,
    ABCParseErrorMidiVoiceIdMissingInPart,
    ABCParseErrorInvalidCaluculatedNoteLength,
    ABCParseErrorInvalidNoteNumber,
    ABCParseErrorIllegalTie,
    ABCParseErrorIllegalOverlay,
    ABCParseErrorUnexpectedEOL,
} ABCParseError;

extern DSLParser *ABCParserCreate(ParseContext *context);

extern Node *ABCParserParseInformation(void *self, const char *filepath, int line, int columnOffset, const char *string);
extern Node *ABCParserParseDirective(void *self, const char *filepath, int line, int columnOffset, const char *string);
extern Node *ABCParserParseTuneBody(void *self, const char *filepath, int line, int columnOffset, const char *string);
extern Node *ABCParserParseIncludeFile(void *self, FileLocation *location, const char *includeFile, char **fullpath);
extern void ABCParserSyntaxError(void *self, FileLocation *location, const char *token);
extern void ABCParserUnExpectedEOL(void *_self, FileLocation *location);
extern bool ABCParserIsLineBreak(void *self, char c);
extern bool ABCParserIsDecoration(void *self, char c);

typedef enum {
    ABCParserEventRefernceNumber,
    ABCParserEventInstLineBreak,
    ABCParserEventInstDecoration,
    ABCParserEventMacro,
    ABCParserEventRedefinableSymbol,
} ABCParserEvent;

extern void ABCParserNotify(void *self, ABCParserEvent event, void *node);
