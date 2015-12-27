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

extern Node *ABCParserParseInformation(void *self, const char *filepath, int line, const char *string);
extern Node *ABCParserParseIncludeFile(void *self, FileLocation *location, const char *includeFile, char **fullpath);
extern void ABCParserSyntaxError(void *self, FileLocation *location, const char *token);
extern void ABCParserSetLineBreak(void *self, char c);
extern bool ABCParserIsLineBreak(void *self, char c);
extern void ABCParserSetDecoration(void *self, char c);
extern bool ABCParserIsDecoration(void *self, char c);
extern void ABCParserSetMacro(void *_self, char *target, char *replacement);
