#pragma once

#include "DSLParser.h"
#include "ParseInfo.h"

typedef enum {
    MMLParseErrorUnsupportedFileTypeInclude = ParseErrorKindMML,
    MMLParseErrorCircularFileInclude,
    MMLParseErrorIncludeFileNotFound,
    MMLParseErrorUnexpectedEOF,
    MMLParseErrorMacroRedefined,
    MMLParseErrorDuplicatedMacroArguments,
    MMLParseErrorUndefinedMacroSymbol,
    MMLParseErrorMacroArgumentsMissing,
    MMLParseErrorWrongNumberOfMacroArguments,
    MMLParseErrorUndefinedMacroArgument,
    MMLParseErrorCircularMacroReference,
    MMLParseErrorIllegalStateWithTimebase,
    MMLParseErrorInvalidTimebase,
    MMLParseErrorIllegalStateWithTitle,
    MMLParseErrorIllegalStateWithCopyright,
    MMLParseErrorIllegalStateWithMarker,
    MMLParseErrorIllegalStateWithVelocityReverse,
    MMLParseErrorIllegalStateWithOctaveReverse,
    MMLParseErrorInvalidChannel,
    MMLParseErrorInvalidBankSelect,
    MMLParseErrorInvalidProgramChange,
    MMLParseErrorInvalidVolume,
    MMLParseErrorInvalidChorus,
    MMLParseErrorInvalidReverb,
    MMLParseErrorInvalidExpression,
    MMLParseErrorInvalidPan,
    MMLParseErrorInvalidDetune,
    MMLParseErrorInvalidTempo,
    MMLParseErrorIllegalStateWithNoteLength,
    MMLParseErrorIllegalStateWithRest,
    MMLParseErrorInvalidOctave,
    MMLParseErrorInvalidTranspose,
    MMLParseErrorIllegalStateWithTie,
    MMLParseErrorIllegalStateWithLength,
    MMLParseErrorInvalidLength,
    MMLParseErrorInvalidGatetime,
    MMLParseErrorInvalidVelocity,
    MMLParseErrorIllegalStateWithTrackChange,
    MMLParseErrorIllegalStateWithTuplet,
    MMLParseErrorIllegalStateWithRepeat,
    MMLParseErrorIllegalStateWithRepeatBreak,
    MMLParseErrorIllegalStateWithChord,
} MMLParseError;

extern DSLParser *MMLParserCreate(ParseContext *context);
extern char *MMLParserGetCurrentFilepath(void *self);
extern void MMLParserSetCurrentFilepath(void *self, const char *filepath);
extern void MMLParserSyntaxError(void *self, FileLocation *location, const char *token);
extern void MMLParserUnExpectedEOF(void *self, FileLocation *location);
