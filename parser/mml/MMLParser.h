#pragma once

#include "DSLParser.h"
#include "ParseInfo.h"

typedef enum {
    MMLParseErrorUnsupportedFileTypeInclude = ParseErrorKindMML,
    MMLParseErrorCircularFileInclude,
    MMLParseErrorIncludeFileNotFound,
    MMLParseErrorUnexpectedEOF,
} MMLParseError;

extern DSLParser *MMLParserCreate(ParseContext *context);
extern void MMLParserParseIncludeFile(void *self, int line, int column, const char *includeFile);
extern bool MMLParserPopPreviousFile(void *self);
extern char *MMLParserGetCurrentFilepath(void *self);
extern void MMLParserSyntaxError(void *self, FileLocation *location, const char *token);
extern void MMLParserUnExpectedEOF(void *self, FileLocation *location);
