#pragma once

#include "SequenceBuilder.h"
#include "NAArray.h"
#include <stdbool.h>
#include <stdarg.h>

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

typedef struct _ParseLocation {
    char *filepath;
    int line;
    int column;
} ParseLocation;

typedef struct _ParseError {
    ParseLocation location;
    int code;
    char *infos[4];
} ParseError;

typedef struct _ParseInfo {
    NAArray *filepaths;
    NAArray *errors;
} ParseInfo;

extern ParseError *ParseErrorCreate(const ParseLocation *location, int code, ...);
extern ParseError *ParseErrorCreateWithArgs(const ParseLocation *location, int code, va_list argList);
extern void ParseErrorDestroy(ParseError *self);

extern ParseInfo *ParseInfoCreate();
extern ParseInfo *ParseInfoRetain(ParseInfo *self);
extern void ParseInfoRelease(ParseInfo *self);

typedef struct _ParserCallbacks {
    void (*onReadFile)(void *receiver, const char *filepath);
    void (*onParseError)(void *receiver, const ParseError *error);
} ParserCallbacks;

typedef struct _DSLParser {
    bool (*parseFile)(void *self, const char *filepath);
    void (*destroy)(void *self);
} DSLParser;

typedef DSLParser *(*DSLParserFactory)(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver);

typedef struct _Parser Parser;

extern Parser *ParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver);
extern void ParserDestroy(Parser *self);
extern bool ParserParseFile(Parser *self, const char *filepath, void **sequence, ParseInfo **info);
extern const char *ParseError2String(const ParseError *error);

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
    default:
       break;
    }

    return "Unknown error";
#undef CASE
}
