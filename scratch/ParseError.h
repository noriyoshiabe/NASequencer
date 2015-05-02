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
    ParseErrorNoteInvalidTaplet,
    ParseErrorNoteInvalidTimeSign,
    ParseErrorNoteIllegalOctaveShift,
    ParseErrorNoteInvalidKeySign,
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
    CASE(ParseErrorNoteInvalidTaplet);
    CASE(ParseErrorNoteInvalidTimeSign);
    CASE(ParseErrorNoteIllegalOctaveShift);
    CASE(ParseErrorNoteInvalidKeySign);
    }
    return "Unknown error type";
#undef CASE
}
