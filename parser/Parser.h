#pragma once

#include "Sequence.h"
#include "NAArray.h"

#include <stdbool.h>

typedef enum {
    ParseErrorKindUnsupportedFileType,
    ParseErrorKindFileNotFound,
    ParseErrorKindSyntaxError,
    ParseErrorKindInvalidValue,

    ParseErrorKindCircularFileInclude,
    ParseErrorKindUnexpectedEnd,
    ParseErrorKindDuplicatePatternIdentifier,
} ParseErrorKind;

typedef struct _ParseLocation {
    int line;
    int column;
    const char *filepath;
} ParseLocation;

typedef struct _ParseError {
    ParseErrorKind kind;
    ParseLocation location;
} ParseError;

typedef struct _ParseResult {
    ParseError error;
    Sequence *sequence;
    NAArray *filepaths;
} ParseResult;

extern bool ParserParseFile(const char *filepath, ParseResult *result);

static inline const char *ParseErrorKind2String(ParseErrorKind kind)
{
#define CASE(kind) case kind: return &(#kind[14])
    switch (kind) {
    CASE(ParseErrorKindUnsupportedFileType);
    CASE(ParseErrorKindFileNotFound);
    CASE(ParseErrorKindSyntaxError);
    CASE(ParseErrorKindInvalidValue);

    CASE(ParseErrorKindCircularFileInclude);
    CASE(ParseErrorKindUnexpectedEnd);
    CASE(ParseErrorKindDuplicatePatternIdentifier);
    default:
       break;
    }

    return "Unknown error kind";
#undef CASE
}

typedef struct _Parser Parser;
struct _Parser {
    bool (*parseFile)(void *parser, const char *filepath);
    void (*destroy)(void *parser);
};

typedef Parser *(*ParserFactory)(ParseResult *result);
