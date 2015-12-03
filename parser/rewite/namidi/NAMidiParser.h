#pragma once

#include "DSLParser.h"

typedef enum {
    NAMidiParseErrorInvalidResolution = 0,
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
    NAMidiParseErrorTooManyNoteParams,
    NAMidiParseErrorUnsupportedFileTypeInclude,
    NAMidiParseErrorIncludeFileNotFound,
    NAMidiParseErrorCircularFileInclude,
    NAMidiParseErrorPatternMissing,
    NAMidiParseErrorDuplicatePatternIdentifier,
    NAMidiParseErrorCircularPatternReference,
} NAMidiParseError;

extern DSLParser *NAMidiParserCreate(ParseContext *context);
