#pragma once

#include "ParseContext.h"

#include <stdio.h>
#include <stdbool.h>

typedef struct _MMLPreprocessor MMLPreprocessor;

extern MMLPreprocessor *MMLPreprocessorCreate(ParseContext *context);
extern void MMLPreprocessorDestroy(MMLPreprocessor *self);
extern void MMLPreprocessorScanFile(MMLPreprocessor *self, const char *filepath);
extern void MMLPreprocessorIncludeFile(MMLPreprocessor *self, int line, int column, const char *includeFile);
extern bool MMLPreprocessorPopPreviousFile(MMLPreprocessor *self);
extern void MMLPreprocessorAppendMacro(MMLPreprocessor *self, int line, int column, char *difinition);
extern char *MMLPreprocessorExpandMacro(MMLPreprocessor *self, int line, int column, char *string);
