#pragma once

#include "SequenceBuilder.h"
#include <stdbool.h>

typedef enum {
    ParseErrorKindGeneral,
    ParseErrorKindNAMidi,
    ParseErrorKindABC,
    ParseErrorKindMML,
} ParseErrorKind;

typedef enum {
    GeneralParseErrorUnsupportedFileType,
    GeneralParseErrorFileNotFound,
    GeneralParseErrorSyntaxError,
    GeneralParseErrorInvalidValue,
    GeneralParseErrorCircularFileInclude,
    GeneralParseErrorInvalidNoteRange,
} GeneralParseError;

typedef struct _ParseLocation {
    const char *filepath;
    int line;
    int column;
} ParseLocation;

typedef struct _ParseError {
    ParseLocation location;
    ParseErrorKind kind;
    int error;
} ParseError;

typedef struct _Parser {
    bool (*parseFile)(void *self, const char *filepath);
    void (*destroy)(void *self);
} Parser;

typedef struct _ParserCallbacks {
    void (*onReadFile)(void *receiver, const char *filepath);
    void (*onParseError)(void *receiver, ParseError *error);
} ParserCallbacks;

typedef Parser *(*ParserFactory)(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver);

static inline const char *ParseErrorKind2String(ParseErrorKind kind)
{
#define CASE(kind) case kind: return &(#kind[14])
    switch (kind) {
    CASE(ParseErrorKindGeneral);
    CASE(ParseErrorKindNAMidi);
    CASE(ParseErrorKindABC);
    CASE(ParseErrorKindMML);
    default:
       break;
    }

    return "Unknown error kind";
#undef CASE
}

static inline const char *GeneralParseError2String(GeneralParseError error)
{
#define CASE(error) case error: return &(#error[17])
    switch (error) {
    CASE(GeneralParseErrorUnsupportedFileType);
    CASE(GeneralParseErrorFileNotFound);
    CASE(GeneralParseErrorSyntaxError);
    CASE(GeneralParseErrorInvalidValue);
    CASE(GeneralParseErrorCircularFileInclude);
    CASE(GeneralParseErrorInvalidNoteRange);
    default:
       break;
    }

    return "Unknown error";
#undef CASE
}
