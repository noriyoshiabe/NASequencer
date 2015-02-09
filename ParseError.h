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


static inline const char *ParseError2String(ParseErrorKind kind)
{
#define ERR2STR_AND_RETURN(e, kind) if (e == kind) return #e
    ERR2STR_AND_RETURN(PARSE_ERROR_NOERROR, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_FILE_NOT_FOUND, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_INIT_ERROR, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_SYNTAX_ERROR, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_RESOLUTION_REDEFINED, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_TITLE_REDEFINED, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_NOTE_BLOCK_MISSING, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_INVALID_STEP, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_INVALID_TIE, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_INVALID_TIME_SIGN, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_INVALID_MSB, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_INVALID_LSB, kind);
    ERR2STR_AND_RETURN(PARSE_ERROR_INVALID_PROGRAM_NO, kind);
#undef ERR2STR_AND_RETURN
    return "Unknown error kind";
}
