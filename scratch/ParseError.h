#pragma once

typedef enum {
    ParseErrorFileNotFound,
    ParseErrorInitError,
    ParseErrorSyntaxError,

    ParseErrorNoteIllegalSharp,
    ParseErrorNoteIllegalFlat,
    ParseErrorNoteIllegalNatural,
    ParseErrorNoteIllegalOctaveUp,
    ParseErrorNoteIllegalOctaveDown,
    ParseErrorQuantizeInvalidTaplet,
    ParseErrorInvalidTimeSign,
    ParseErrorIllegalOctaveShift,
    ParseErrorInvalidKeySign,
    ParseErrorInvalidTempo,
    ParseErrorInvalidChannel,
    ParseErrorInvalidVelociy,
    ParseErrorInvalidGatetime,
    ParseErrorInvalidOctave,
    ParseErrorInvalidLocation,
    ParseErrorInvalidMeasure,
    ParseErrorPatternMissing,
    ParseErrorPatternCircularReference,
    ParseErrorInvalidOffset,
    ParseErrorInvalidLength,
} ParseError;

static inline const char *ParseError2String(ParseError error)
{
#define CASE(error) case error: return &(#error[10])
    switch (error) {
    CASE(ParseErrorFileNotFound);
    CASE(ParseErrorInitError);
    CASE(ParseErrorSyntaxError);

    CASE(ParseErrorNoteIllegalSharp);
    CASE(ParseErrorNoteIllegalFlat);
    CASE(ParseErrorNoteIllegalNatural);
    CASE(ParseErrorNoteIllegalOctaveUp);
    CASE(ParseErrorNoteIllegalOctaveDown);
    CASE(ParseErrorQuantizeInvalidTaplet);
    CASE(ParseErrorInvalidTimeSign);
    CASE(ParseErrorIllegalOctaveShift);
    CASE(ParseErrorInvalidKeySign);
    CASE(ParseErrorInvalidTempo);
    CASE(ParseErrorInvalidChannel);
    CASE(ParseErrorInvalidVelociy);
    CASE(ParseErrorInvalidGatetime);
    CASE(ParseErrorInvalidOctave);
    CASE(ParseErrorInvalidLocation);
    CASE(ParseErrorInvalidMeasure);
    CASE(ParseErrorPatternMissing);
    CASE(ParseErrorPatternCircularReference);
    CASE(ParseErrorInvalidOffset);
    CASE(ParseErrorInvalidLength);
    }
    return "Unknown error type";
#undef CASE
}
