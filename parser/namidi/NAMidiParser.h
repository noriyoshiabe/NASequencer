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
    NAMidiParseErrorUndefinedMacroSymbol,
    NAMidiParseErrorUnexpectedEOF,
    NAMidiParseErrorInvalidResolution,
    NAMidiParseErrorInvalidTempo,
    NAMidiParseErrorInvalidTimeSign,
    NAMidiParseErrorInvalidChannel,
    NAMidiParseErrorInvalidVoice,
    NAMidiParseErrorInvalidVolume,
    NAMidiParseErrorInvalidPan,
    NAMidiParseErrorInvalidChorus,
    NAMidiParseErrorInvalidReverb,
    NAMidiParseErrorInvalidExpression,
    NAMidiParseErrorInvalidDetune,
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
    NAMidiParseErrorIllegalStateWithInclude,
    NAMidiParseErrorAlreadyDefinedWithResolution,
    NAMidiParseErrorAlreadyDefinedWithTitle,
    NAMidiParseErrorAlreadyDefinedWithCopyright,
    NAMidiParseErrorTooManyNoteParams,
    NAMidiParseErrorPatternMissing,
    NAMidiParseErrorDuplicatePatternIdentifier,
    NAMidiParseErrorCircularPatternReference,
} NAMidiParseError;

extern DSLParser *NAMidiParserCreate(ParseContext *context);
