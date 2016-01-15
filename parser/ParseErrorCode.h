#include "ParseInfo.h"
#include "NAMidiParser.h"
#include "ABCParser.h"
#include "MMLParser.h"

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
    CASE(NAMidiParseErrorUnexpectedEOF);

    CASE(ABCParseErrorUnrecognisedVersion);
    CASE(ABCParseErrorUnsupportedFileTypeInclude);
    CASE(ABCParseErrorCircularFileInclude);
    CASE(ABCParseErrorIncludeFileNotFound);
    CASE(ABCParseErrorIllegalStateWithTitle);
    CASE(ABCParseErrorIllegalStateWithKey);
    CASE(ABCParseErrorDuplicatedKeyTonic);
    CASE(ABCParseErrorDuplicatedKeyMode);
    CASE(ABCParseErrorInvalidTranspose);
    CASE(ABCParseErrorInvalidOctave);
    CASE(ABCParseErrorInvalidKeyMode);
    CASE(ABCParseErrorInvalidKey);
    CASE(ABCParseErrorInvalidMeter);
    CASE(ABCParseErrorInvalidUnitNoteLength);
    CASE(ABCParseErrorInvalidBeatUnit);
    CASE(ABCParseErrorInvalidBeatCount);
    CASE(ABCParseErrorInvalidTempo);
    CASE(ABCParseErrorIllegalStateWithParts);
    CASE(ABCParseErrorDuplicatedParts);
    CASE(ABCParseErrorIllegalStateWithInclude);
    CASE(ABCParseErrorIllegalStateWithVoice);
    CASE(ABCParseErrorIllegalStateWithTuneBody);
    CASE(ABCParseErrorInvalidNoteLength);
    CASE(ABCParseErrorInvalidRepeat);
    CASE(ABCParseErrorInvalidNthRepeat);
    CASE(ABCParseErrorIllegalStateWithMidiVoice);
    CASE(ABCParseErrorDuplicatedMidiVoiceId);
    CASE(ABCParseErrorInvalidMidiInstrument);
    CASE(ABCParseErrorMidiVoiceIdMissingInTune);
    CASE(ABCParseErrorMidiVoiceIdMissingInPart);
    CASE(ABCParseErrorInvalidCaluculatedNoteLength);
    CASE(ABCParseErrorInvalidNoteNumber);
    CASE(ABCParseErrorIllegalTie);
    CASE(ABCParseErrorIllegalOverlay);

    CASE(MMLParseErrorUnsupportedFileTypeInclude);
    CASE(MMLParseErrorCircularFileInclude);
    CASE(MMLParseErrorIncludeFileNotFound);
    CASE(MMLParseErrorUnexpectedEOF);
    CASE(MMLParseErrorMacroRedefined);
    CASE(MMLParseErrorDuplicatedMacroArguments);
    CASE(MMLParseErrorUndefinedMacroSymbol);
    CASE(MMLParseErrorMacroArgumentsMissing);
    CASE(MMLParseErrorWrongNumberOfMacroArguments);
    CASE(MMLParseErrorUndefinedMacroArgument);
    CASE(MMLParseErrorCircularMacroReference);
    CASE(MMLParseErrorIllegalStateWithTimebase);
    CASE(MMLParseErrorInvalidTimebase);
    CASE(MMLParseErrorIllegalStateWithTitle);
    CASE(MMLParseErrorIllegalStateWithCopyright);
    CASE(MMLParseErrorIllegalStateWithMarker);
    CASE(MMLParseErrorIllegalStateWithVelocityReverse);
    CASE(MMLParseErrorIllegalStateWithOctaveReverse);
    CASE(MMLParseErrorInvalidChannel);
    CASE(MMLParseErrorInvalidBankSelect);
    CASE(MMLParseErrorInvalidProgramChange);
    CASE(MMLParseErrorInvalidVolume);
    CASE(MMLParseErrorInvalidChorus);
    CASE(MMLParseErrorInvalidReverb);
    CASE(MMLParseErrorInvalidExpression);
    CASE(MMLParseErrorInvalidPan);
    CASE(MMLParseErrorInvalidDetune);
    CASE(MMLParseErrorInvalidTempo);
    CASE(MMLParseErrorIllegalStateWithNoteLength);
    CASE(MMLParseErrorIllegalStateWithRest);
    CASE(MMLParseErrorInvalidOctave);
    CASE(MMLParseErrorInvalidTranspose);
    CASE(MMLParseErrorIllegalStateWithTie);
    CASE(MMLParseErrorIllegalStateWithLength);
    CASE(MMLParseErrorInvalidLength);
    CASE(MMLParseErrorInvalidGatetime);
    CASE(MMLParseErrorInvalidVelocity);
    CASE(MMLParseErrorIllegalStateWithTrackChange);
    CASE(MMLParseErrorIllegalStateWithTuplet);
    CASE(MMLParseErrorIllegalStateWithRepeat);
    CASE(MMLParseErrorIllegalStateWithRepeatBreak);
    CASE(MMLParseErrorIllegalStateWithChord);

    default:
       break;
    }

    return "Unknown error";
#undef CASE
}
