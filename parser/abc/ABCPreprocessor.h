#pragma once

#include <stdio.h>

typedef struct _ABCPreprocessor ABCPreprocessor;

extern ABCPreprocessor *ABCPreprocessorCreate();
extern void ABCPreprocessorDestroy(ABCPreprocessor *self);
extern void ABCPreprocessorProcess(ABCPreprocessor *self, FILE *input, const char *filepath);
extern FILE *ABCPreprocessorGetStream(ABCPreprocessor *self, const char *filepath);
