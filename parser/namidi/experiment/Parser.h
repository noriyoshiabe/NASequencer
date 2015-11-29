#pragma once

#include "Node.h"

typedef struct _Parser Parser;

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

extern void ParserError(void *parser, FileLocation *location, int code, ...);
