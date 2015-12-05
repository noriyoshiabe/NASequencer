#include "ParseInfo.h"
#include "NAMidiParser.h"

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
    CASE(NAMidiParseErrorResolutionAlreadyDefined);
    CASE(NAMidiParseErrorTitleAlreadyDefined);
    CASE(NAMidiParseErrorTooManyNoteParams);
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
