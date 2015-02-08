#pragma once

#include "Expression.h"

typedef enum {
    PARSE_ERROR_NOERROR,

    PARSE_ERROR_FILE_NOT_FOUND,
    PARSE_ERROR_INIT_ERROR,
    PARSE_ERROR_SYNTAX_ERROR,

    PARSE_ERROR_RESOLUTION_REDEFINED,
    PARSE_ERROR_TITLE_REDEFINED,
    PARSE_ERROR_NOTE_BLOCK_MISSING,
    PARSE_ERROR_INVALID_STEP,
    PARSE_ERROR_INVALID_TIE,
    PARSE_ERROR_INVALID_TIME_SIGN,
    PARSE_ERROR_INVALID_MSB,
    PARSE_ERROR_INVALID_LSB,
    PARSE_ERROR_INVALID_PROGRAM_NO,
} ParseErrorKind;

typedef struct _ParseError {
    ParseErrorKind kind;
    const char *filepath;
    const char *message;
    Location location;
} ParseError;
