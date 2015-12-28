#pragma once

#include <stdio.h>

typedef struct _ABCPreprocessor ABCPreprocessor;

extern ABCPreprocessor *ABCPreprocessorCreate();
extern void ABCPreprocessorDestroy(ABCPreprocessor *self);
extern void ABCPreprocessorSetMacro(ABCPreprocessor *self, char *target, char *replacement);
extern void ABCPreprocessorSetRedefinableSymbol(ABCPreprocessor *self, char *symbol, char *replacement);
extern char *ABCPreprocessorPreprocessTuneBody(ABCPreprocessor *self, const char *tuneBody);
