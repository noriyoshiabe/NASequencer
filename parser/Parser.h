#pragma once

#include "Sequence.h"
#include "NASet.h"

#include <stdbool.h>

#define ParserMeasureMax 9999

typedef enum {
    StatementTypeResolution,
    StatementTypeTitle,
    StatementTypeTempo,
    StatementTypeTimeSign,
    StatementTypeMeasure,
    StatementTypeMarker,
    StatementTypePattern,
    StatementTypePatternDefine,
    StatementTypeEnd,
    StatementTypeTrack,
    StatementTypeChannel,
    StatementTypeVoice,
    StatementTypeVolume,
    StatementTypePan,
    StatementTypeChorus,
    StatementTypeReverb,
    StatementTypeTranspose,
    StatementTypeKey,
    StatementTypeNote,
    StatementTypeRest,
    StatementTypeInclude,

    StatementTypeCount,
} StatementType;

typedef enum {
    ParseErrorKindUnsupportedFileType,
    ParseErrorKindFileNotFound,
    ParseErrorKindSyntaxError,
    ParseErrorKindInvalidValue,
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

typedef struct _StatementHandler StatementHandler;

typedef struct _ParseContext {
    StatementHandler *handler;
    void *receiver;
    ParseLocation location;
    NASet *fileSet;
    ParseResult *result;
} ParseContext;

typedef bool (*StatementHandlerProcessFunction)(void *receiver, ParseContext *context, StatementType type, ...);
typedef void (*StatementHandlerErrorFunction)(void *receiver, ParseContext *context, ParseError *error);

struct _StatementHandler {
    StatementHandlerProcessFunction process;
    StatementHandlerErrorFunction error;
};

extern bool ParserParseFile(const char *filepath, ParseResult *result);
extern bool ParserParseFileWithContext(const char *filepath, ParseContext *context);

static inline const char *StatementType2String(StatementType type)
{
#define CASE(type) case type: return &(#type[13])
    switch (type) {
    CASE(StatementTypeResolution);
    CASE(StatementTypeTitle);
    CASE(StatementTypeTempo);
    CASE(StatementTypeTimeSign);
    CASE(StatementTypeMeasure);
    CASE(StatementTypeMarker);
    CASE(StatementTypePattern);
    CASE(StatementTypePatternDefine);
    CASE(StatementTypeEnd);
    CASE(StatementTypeTrack);
    CASE(StatementTypeChannel);
    CASE(StatementTypeVoice);
    CASE(StatementTypeVolume);
    CASE(StatementTypePan);
    CASE(StatementTypeChorus);
    CASE(StatementTypeReverb);
    CASE(StatementTypeTranspose);
    CASE(StatementTypeKey);
    CASE(StatementTypeNote);
    CASE(StatementTypeRest);
    CASE(StatementTypeInclude);

    default:
       break;
    }

    return "Unknown statement type";
#undef CASE
}

static inline const char *ParseErrorKind2String(ParseErrorKind kind)
{
#define CASE(kind) case kind: return &(#kind[14])
    switch (kind) {
    CASE(ParseErrorKindUnsupportedFileType);
    CASE(ParseErrorKindFileNotFound);
    CASE(ParseErrorKindSyntaxError);
    CASE(ParseErrorKindInvalidValue);

    default:
       break;
    }

    return "Unknown error kind";
#undef CASE
}
