#pragma once

#include <NAType.h>
#include "ParseError.h"
#include "Sequence.h"

typedef struct _ParseContext {
    NAType _;
    const char *filepath;
    Sequence *sequence;
    CFMutableDictionaryRef patterns;
    ParseError error;
} ParseContext;

extern ParseContext *ParseContextParse(const char *filepath);

NAExportClass(ParseContext);


typedef struct _ParseContextViewVtbl {
    void (*render)(void *self, ParseContext *context);
} ParseContextViewVtbl;

extern void ParseContextViewRender(void *self, ParseContext *context);

NAExportClass(ParseContextView);
