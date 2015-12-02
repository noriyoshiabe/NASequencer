#pragma once

#include "FileLocation.h"
#include "NAArray.h"

typedef enum {
    ParseErrorKindGeneral = 1000,
    ParseErrorKindNAMidi = 2000,
    ParseErrorKindABC = 3000,
    ParseErrorKindMML = 4000,
} ParseErrorKind;

typedef enum {
    GeneralParseErrorUnsupportedFileType = ParseErrorKindGeneral,
    GeneralParseErrorFileNotFound,
    GeneralParseErrorSyntaxError,
} GeneralParseError;

typedef struct _ParseError {
    FileLocation location;
    int code;
    NAArray *infos;
} ParseError;

typedef struct _ParseResult {
    void *sequence;
    NAArray *filepaths;
    NAArray *errors;
} ParseResult;

extern ParseError *ParseErrorCreate();
extern void ParseErrorDestroy(ParseError *self);

extern ParseResult *ParseResultCreate();
extern ParseResult *ParseResultRetain(ParseResult *self);
extern void ParseResultRelease(ParseResult *self);
