#pragma once

#include "Parser.h"

#include <stdarg.h>

typedef enum {
    StatementTypeResolution,
    StatementTypeTitle,
    StatementTypeTempo,
    StatementTypeTimeSign,
    StatementTypeMarker,
    StatementTypePattern,
    StatementTypePatternDefine,
    StatementTypeContext,
    StatementTypeContextDefault,
    StatementTypeContextEnd,
    StatementTypeEnd,
    StatementTypeChannel,
    StatementTypeVoice,
    StatementTypeSynth,
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
    CASE(StatementTypeMarker);
    CASE(StatementTypePattern);
    CASE(StatementTypePatternDefine);
    CASE(StatementTypeContext);
    CASE(StatementTypeContextDefault);
    CASE(StatementTypeContextEnd);
    CASE(StatementTypeEnd);
    CASE(StatementTypeChannel);
    CASE(StatementTypeVoice);
    CASE(StatementTypeSynth);
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
