#pragma once

#include <stdbool.h>

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
    StatementTypeChrous,
    StatementTypeReverb,
    StatementTypeTranspose,
    StatementTypeKey,
    StatementTypeNote,
    StatementTypeRest,
    StatementTypeInclude,

    StatementTypeCount,
} StatementType;

typedef struct _ParseLocation {
    const char *filepath;
    int line;
    int column;
} ParseLocation;

typedef bool (*ParserCallback)(void *context, ParseLocation *location, StatementType type, ...);
typedef void (*ParserErrorCallback)(void *context, ParseLocation *location, const char *message);

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
    CASE(StatementTypeChrous);
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
