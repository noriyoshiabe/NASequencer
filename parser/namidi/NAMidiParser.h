#pragma once

#include "DSLParser.h"
#include "ParseInfo.h"

typedef enum {
    NAMidiParseErrorUnsupportedFileTypeInclude = ParseErrorKindNAMidi,
    NAMidiParseErrorIncludeFileNotFound,
    NAMidiParseErrorCircularFileInclude,
    NAMidiParseErrorMacroRedefined,
    NAMidiParseErrorDuplicatedMacroArguments,
    NAMidiParseErrorUndefinedMacroArgument,
    NAMidiParseErrorCircularMacroReference,
    NAMidiParseErrorMacroArgumentsMissing,
    NAMidiParseErrorWrongNumberOfMacroArguments,
    NAMidiParseErrorUnexpectedEOF,
    NAMidiParseErrorUnexpectedEOL,
    NAMidiParseErrorInvalidResolution,
    NAMidiParseErrorInvalidTempo,
    NAMidiParseErrorInvalidTimeSign,
    NAMidiParseErrorInvalidChannel,
    NAMidiParseErrorInvalidBankNumber,
    NAMidiParseErrorInvalidProgramNumber,
    NAMidiParseErrorInvalidVolume,
    NAMidiParseErrorInvalidPan,
    NAMidiParseErrorInvalidChorus,
    NAMidiParseErrorInvalidReverb,
    NAMidiParseErrorInvalidExpression,
    NAMidiParseErrorInvalidPitch,
    NAMidiParseErrorInvalidDetune,
    NAMidiParseErrorInvalidPitchSense,
    NAMidiParseErrorInvalidTranspose,
    NAMidiParseErrorInvalidKeySign,
    NAMidiParseErrorInvalidStep,
    NAMidiParseErrorInvalidNoteNumber,
    NAMidiParseErrorInvalidOctave,
    NAMidiParseErrorInvalidGatetime,
    NAMidiParseErrorInvalidVelocity,
    NAMidiParseErrorIllegalStateWithResolution,
    NAMidiParseErrorIllegalStateWithTitle,
    NAMidiParseErrorIllegalStateWithCopyright,
    NAMidiParseErrorAlreadyDefinedWithResolution,
    NAMidiParseErrorAlreadyDefinedWithTitle,
    NAMidiParseErrorAlreadyDefinedWithCopyright,
    NAMidiParseErrorTooManyNoteParams,
    NAMidiParseErrorPatternMissing,
    NAMidiParseErrorDuplicatePatternIdentifier,
    NAMidiParseErrorCircularPatternReference,
} NAMidiParseError;

extern DSLParser *NAMidiParserCreate(ParseContext *context);
extern char *NAMidiParserGetCurrentFilepath(void *self);
extern void NAMidiParserSetCurrentFilepath(void *self, const char *filepath);
extern void NAMidiParserSyntaxError(void *self, FileLocation *location, const char *token);
extern void NAMidiParserUnExpectedEOF(void *self, FileLocation *location);
