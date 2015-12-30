#include "ParseInfo.h"
#include "NAMidiParser.h"
#include "ABCParser.h"

static inline const char *ParseErrorCode2String(int code)
{
#define CASE(code) case code: return #code;
    switch (code) {
    CASE(GeneralParseErrorUnsupportedFileType);
    CASE(GeneralParseErrorFileNotFound);
    CASE(GeneralParseErrorSyntaxError);

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
    CASE(NAMidiParseErrorIllegalStateWithResolution);
    CASE(NAMidiParseErrorIllegalStateWithTitle);
    CASE(NAMidiParseErrorIllegalStateWithInclude);
    CASE(NAMidiParseErrorIllegalStateWithDefine);
    CASE(NAMidiParseErrorIllegalStateWithContext);
    CASE(NAMidiParseErrorAlreadyDefinedWithResolution);
    CASE(NAMidiParseErrorAlreadyDefinedWithTitle);
    CASE(NAMidiParseErrorTooManyNoteParams);
    CASE(NAMidiParseErrorUnsupportedFileTypeInclude);
    CASE(NAMidiParseErrorIncludeFileNotFound);
    CASE(NAMidiParseErrorCircularFileInclude);
    CASE(NAMidiParseErrorPatternMissing);
    CASE(NAMidiParseErrorDuplicatePatternIdentifier);
    CASE(NAMidiParseErrorCircularPatternReference);

    CASE(ABCParseErrorUnrecognisedVersion);
    CASE(ABCParseErrorUnsupportedFileTypeInclude);
    CASE(ABCParseErrorCircularFileInclude);
    CASE(ABCParseErrorIncludeFileNotFound);
    CASE(ABCParseErrorIllegalStateWithTitle);
    CASE(ABCParseErrorIllegalStateWithKey);

    default:
       break;
    }

    return "Unknown error";
#undef CASE
}
