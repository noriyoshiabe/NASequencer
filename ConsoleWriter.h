#pragma once

#include <NAType.h>
#include "ParseContext.h"

typedef struct _ConsoleWriter ConsoleWriter;
NAExportClass(ConsoleWriter);

extern void ConsoleWriterWrite(ConsoleWriter *self, ParseContext *parseContext);
