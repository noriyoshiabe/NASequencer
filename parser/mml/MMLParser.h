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
