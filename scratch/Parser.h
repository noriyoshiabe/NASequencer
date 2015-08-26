#pragma once

#include <stdbool.h>
#include <stdarg.h>

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
    ParseErrorSyntaxError,
    ParseErrorInvalidValue,
} ParseError;

typedef enum {
    SyntaxNAMidi,
} Syntax;

typedef struct _ParseLocation {
    int line;
    int column;
} ParseLocation;

typedef struct _StatementHandler {
    bool (*process)(void *self, ParseLocation *location, StatementType type, va_list argList);
    void (*error)(void *self, ParseLocation *location, ParseError error);
} StatementHandler;

typedef struct _Parser Parser;

extern Parser *ParserCreate(Syntax syntax, void *handler);
extern void ParserDestroy(Parser *self);
extern void ParserScanString(Parser *self, char *line);

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

static inline const char *ParseError2String(ParseError error)
{
#define CASE(error) case error: return &(#error[10])
    switch (error) {
    CASE(ParseErrorSyntaxError);
    CASE(ParseErrorInvalidValue);

    default:
       break;
    }

    return "Unknown error type";
#undef CASE
}
