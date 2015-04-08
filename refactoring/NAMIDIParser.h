#pragma once

#include <stdbool.h>
#include "AbstractSequence.h"

typedef struct _ParseContext {
    const char *filepath;
    Sequence *sequence;
    int16_t resolution;
    const char *title;
    int32_t step;
    int32_t tick;
    int32_t channel;
    int32_t velocity;
    int32_t gatetime;
    int32_t octave;
    int32_t length;
    void *option;
} ParseContext;

typedef enum {
    PARSE_ERROR_FILE_NOT_FOUND,
    PARSE_ERROR_INIT_ERROR,
    PARSE_ERROR_SYNTAX_ERROR,

    PARSE_ERROR_RESOLUTION_REDEFINED,
    PARSE_ERROR_TITLE_REDEFINED,
    PARSE_ERROR_NOTE_BLOCK_MISSING,
    PARSE_ERROR_NOTE_BLOCK_STEP_MISSING,
    PARSE_ERROR_NOTE_BLOCK_STEP_REDEFINED,
    PARSE_ERROR_PATTERN_MISSING,
    PARSE_ERROR_INVALID_RESOLUTION,
    PARSE_ERROR_INVALID_STEP,
    PARSE_ERROR_INVALID_TIE,
    PARSE_ERROR_INVALID_TEMPO,
    PARSE_ERROR_INVALID_TIME_SIGN,
    PARSE_ERROR_INVALID_MSB,
    PARSE_ERROR_INVALID_LSB,
    PARSE_ERROR_INVALID_PROGRAM_NO,
    PARSE_ERROR_INVALID_CHANNEL,
    PARSE_ERROR_INVALID_VELOCITY,
    PARSE_ERROR_INVALID_OCTAVE,
    PARSE_ERROR_INVALID_NOTE_NO,
    PARSE_ERROR_INVALID_QUANTIZE,
} ParseErrorKind;

typedef struct _ParseError {
    ParseErrorKind kind;
    const char *filepath;
    const char *message;
    ParseLocation location;
} ParseError;

typedef bool (*ExpressionParseFunction)(Expression *expression, ParseContext *context, void *value, ParseError **error);
extern void registerExpressionParser(ExpressionParseFunction parseFunction);

extern ParseContext *ParseContextCreate(void *sequence);
extern ParseContext *ParseContextCreateLocal(const ParseContext *from);
extern void ParseContextDestroy(ParseContext *context);
extern ParseError *ParseErrorCreate();
extern ParseError *ParseErrorCreateWithError(ParseErrorKind kind, const char *filepath, const char *message, ParseLocation location);
extern void ParseErrorDestroy(ParseError *error);

extern Sequence *NAMidiParserParse(const char *filepath, ParseError **error);

#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))
