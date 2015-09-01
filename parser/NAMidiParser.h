#pragma once

#include "Parser.h"

#include <stdarg.h>

extern Parser *NAMidiParserCreate();

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

typedef struct _NAMidiParser NAMidiParser;

extern bool NAMidiParserProcess(NAMidiParser *self, int line, int column, StatementType type, va_list argList);
extern void NAMidiParserError(NAMidiParser *self, int line, int column, ParseErrorKind errorKind);

extern Parser *NAMidiParserCreate(ParseResult *result);

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
