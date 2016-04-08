#pragma once

#include "ParseContext.h"
#include "NAStringBuffer.h"

#include <stdio.h>
#include <stdbool.h>

typedef struct _NAMidiPreprocessor NAMidiPreprocessor;

extern NAMidiPreprocessor *NAMidiPreprocessorCreate(ParseContext *context);
extern void NAMidiPreprocessorDestroy(NAMidiPreprocessor *self);
extern FILE *NAMidiPreprocessorScanFile(NAMidiPreprocessor *self, const char *filepath);
extern void NAMidiPreprocessorIncludeFile(NAMidiPreprocessor *self, int line, int column, const char *includeFile);
extern bool NAMidiPreprocessorPopPreviousFile(NAMidiPreprocessor *self);
extern void NAMidiPreprocessorAppendMacro(NAMidiPreprocessor *self, int line, int column, char *difinition);
extern void NAMidiPreprocessorRemoveMacro(NAMidiPreprocessor *self, int line, int column, char *identifier);
extern char *NAMidiPreprocessorExpandMacro(NAMidiPreprocessor *self, int line, int column, char *string);
extern void NAMidiPreprocessorConsumeExpandingChar(NAMidiPreprocessor *self, int length);
extern void NAMidiPreprocessorSyntaxError(NAMidiPreprocessor *self, int line, int column, const char *token);
extern void NAMidiPreprocessorUnexpectedEOF(NAMidiPreprocessor *self, int line, int column);

#define YY_DECL int NAMidi_preprocessor_lex (yyscan_t yyscanner, FILE *stream)
