#pragma once

#include "DSLParser.h"
#include "ParseInfo.h"

typedef enum {
    MMLParseErrorUnsupportedFileTypeInclude = ParseErrorKindMML,
    MMLParseErrorCircularFileInclude,
    MMLParseErrorIncludeFileNotFound,
    MMLParseErrorUnexpectedEOF,
    MMLParseErrorMacroRedefined,
    MMLParseErrorUndefinedMacroSymbol,
    MMLParseErrorMacroArgumentsMissing,
    MMLParseErrorWrongNumberOfMacroArguments,
    MMLParseErrorUndefinedMacroArgument,
} MMLParseError;

extern DSLParser *MMLParserCreate(ParseContext *context);
extern char *MMLParserGetCurrentFilepath(void *self);
extern void MMLParserSetCurrentFilepath(void *self, const char *filepath);
extern void MMLParserSyntaxError(void *self, FileLocation *location, const char *token);
extern void MMLParserUnExpectedEOF(void *self, FileLocation *location);
