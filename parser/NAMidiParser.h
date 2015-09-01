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

bool NAMidiParserProcess(NAMidiParser *self, int line, int column, StatementType type, va_list argList);
void NAMidiParserError(NAMidiParser *self, int line, int column, ParseErrorKind errorKind);

extern Parser *NAMidiParserCreate();
