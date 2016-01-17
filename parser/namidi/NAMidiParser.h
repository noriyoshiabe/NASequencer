#pragma once

#include "DSLParser.h"
#include "ParseInfo.h"

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
    NAMidiParseErrorIllegalStateWithResolution,
    NAMidiParseErrorIllegalStateWithTitle,
    NAMidiParseErrorIllegalStateWithCopyright,
    NAMidiParseErrorIllegalStateWithInclude,
    NAMidiParseErrorIllegalStateWithDefine,
    NAMidiParseErrorIllegalStateWithContext,
    NAMidiParseErrorAlreadyDefinedWithResolution,
    NAMidiParseErrorAlreadyDefinedWithTitle,
    NAMidiParseErrorAlreadyDefinedWithCopyright,
    NAMidiParseErrorTooManyNoteParams,
    NAMidiParseErrorUnsupportedFileTypeInclude,
    NAMidiParseErrorIncludeFileNotFound,
    NAMidiParseErrorCircularFileInclude,
    NAMidiParseErrorPatternMissing,
    NAMidiParseErrorDuplicatePatternIdentifier,
    NAMidiParseErrorCircularPatternReference,
    NAMidiParseErrorUnexpectedEOF,
} NAMidiParseError;

extern DSLParser *NAMidiParserCreate(ParseContext *context);
