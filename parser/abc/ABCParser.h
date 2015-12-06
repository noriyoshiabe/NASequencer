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
