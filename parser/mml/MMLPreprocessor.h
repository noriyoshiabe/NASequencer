#pragma once

#include "ParseContext.h"

#include <stdio.h>
#include <stdbool.h>

typedef struct _MMLPreprocessor MMLPreprocessor;

extern MMLPreprocessor *MMLPreprocessorCreate(ParseContext *context);
extern void MMLPreprocessorDestroy(MMLPreprocessor *self);
extern void MMLPreprocessorAppendMacro(MMLPreprocessor *self, char *difinition, int line, int column);
extern char *MMLPreprocessorExpandMacro(MMLPreprocessor *self, char *string, int line, int column);
