#pragma once

#include "DSLParser.h"
#include "ParseInfo.h"

typedef enum {
    ABCParseErrorUnrecognisedVersion = ParseErrorKindABC,
    ABCParseErrorUnsupportedFileTypeInclude,
    ABCParseErrorCircularFileInclude,
    ABCParseErrorIncludeFileNotFound,
} ABCParseError;

extern DSLParser *ABCParserCreate(ParseContext *context);

extern Node *ABCParserParseIncludeFile(void *self, FileLocation *location, const char *includeFile);
extern void ABCParserSyntaxError(void *self, FileLocation *location, const char *token);
extern void ABCParserSetLineBreak(void *self, char c);
extern char ABCParserGetLineBreak(void *self);
