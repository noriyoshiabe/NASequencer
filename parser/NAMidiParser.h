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

extern bool NAMidiParserProcess(void *self, int line, int column, StatementType type, va_list argList);
extern void NAMidiParserError(void *self, int line, int column, ParseErrorKind errorKind);

extern Parser *NAMidiParserCreate(ParseResult *result);
