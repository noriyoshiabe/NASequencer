#pragma once

#include <stdio.h>

typedef struct _ABCPreprocessor ABCPreprocessor;

extern ABCPreprocessor *ABCPreprocessorCreate();
extern void ABCPreprocessorDestroy(ABCPreprocessor *self);
extern FILE *ABCPreprocessorProcess(ABCPreprocessor *self, FILE *fp);
