#pragma once

#include <NAType.h>
#include "ParseError.h"
#include "Sequence.h"

typedef struct _ParseContext {
    NAType _;
    CFStringRef filepath;
    Sequence *sequence;
    CFMutableDictionaryRef patterns;
    ParseError *error;
} ParseContext;

extern ParseContext *ParseContextParse(CFStringRef filepath);

NAExportClass(ParseContext);
