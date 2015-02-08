#pragma once

#include <NAType.h>
#include "ParseError.h"
#include "Sequence.h"

typedef struct _ParseContext {
    NAType _;
    const char *filepath;
    Sequence *sequence;
    CFMutableDictionaryRef patterns;
} ParseContext;

extern ParseContext *ParseContextParse(const char *filepath, ParseError *error);

NAExportClass(ParseContext);
