#pragma once

typedef enum {
    ParseErrorFileNotFound,
    ParseErrorInitError,
    ParseErrorSyntaxError,

    ParseErrorNoteIllegalSharp,
    ParseErrorNoteIllegalFlat,
    ParseErrorNoteIllegalOctaveUp,
    ParseErrorNoteIllegalOctaveDown,
    ParseErrorNoteInvalidTaplet,
    ParseErrorNoteInvalidTimeSign,
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
    CASE(ParseErrorNoteIllegalOctaveUp);
    CASE(ParseErrorNoteIllegalOctaveDown);
    CASE(ParseErrorNoteInvalidTaplet);
    CASE(ParseErrorNoteInvalidTimeSign);
    }
    return "Unknown error type";
#undef CASE
}
