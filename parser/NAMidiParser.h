#pragma once

#include "Parser.h"

#include <stdarg.h>

typedef enum {
    NAMidiStatementTypeResolution,
    NAMidiStatementTypeTitle,
    NAMidiStatementTypeTempo,
    NAMidiStatementTypeTimeSign,
    NAMidiStatementTypeMarker,
    NAMidiStatementTypePattern,
    NAMidiStatementTypePatternDefine,
    NAMidiStatementTypeContext,
    NAMidiStatementTypeContextDefault,
    NAMidiStatementTypeContextEnd,
    NAMidiStatementTypeEnd,
    NAMidiStatementTypeChannel,
    NAMidiStatementTypeVoice,
    NAMidiStatementTypeSynth,
    NAMidiStatementTypeVolume,
    NAMidiStatementTypePan,
    NAMidiStatementTypeChorus,
    NAMidiStatementTypeReverb,
    NAMidiStatementTypeTranspose,
    NAMidiStatementTypeKey,
    NAMidiStatementTypeNote,
    NAMidiStatementTypeRest,
    NAMidiStatementTypeInclude,

    NAMidiStatementTypeCount,
} NAMidiStatementType;

typedef struct _NAMidiParser NAMidiParser;

extern bool NAMidiParserProcess(NAMidiParser *self, int line, int column, NAMidiStatementType type, va_list argList);
extern void NAMidiParserError(NAMidiParser *self, int line, int column, ParseErrorKind errorKind);

extern Parser *NAMidiParserCreate(ParseResult *result);

static inline const char *NAMidiStatementType2String(NAMidiStatementType type)
{
#define CASE(type) case type: return &(#type[13])
    switch (type) {
    CASE(NAMidiStatementTypeResolution);
    CASE(NAMidiStatementTypeTitle);
    CASE(NAMidiStatementTypeTempo);
    CASE(NAMidiStatementTypeTimeSign);
    CASE(NAMidiStatementTypeMarker);
    CASE(NAMidiStatementTypePattern);
    CASE(NAMidiStatementTypePatternDefine);
    CASE(NAMidiStatementTypeContext);
    CASE(NAMidiStatementTypeContextDefault);
    CASE(NAMidiStatementTypeContextEnd);
    CASE(NAMidiStatementTypeEnd);
    CASE(NAMidiStatementTypeChannel);
    CASE(NAMidiStatementTypeVoice);
    CASE(NAMidiStatementTypeSynth);
    CASE(NAMidiStatementTypeVolume);
    CASE(NAMidiStatementTypePan);
    CASE(NAMidiStatementTypeChorus);
    CASE(NAMidiStatementTypeReverb);
    CASE(NAMidiStatementTypeTranspose);
    CASE(NAMidiStatementTypeKey);
    CASE(NAMidiStatementTypeNote);
    CASE(NAMidiStatementTypeRest);
    CASE(NAMidiStatementTypeInclude);

    default:
       break;
    }

    return "Unknown statement type";
#undef CASE
}
