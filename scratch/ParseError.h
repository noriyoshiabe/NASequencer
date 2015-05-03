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
    }
    return "Unknown error type";
#undef CASE
}
