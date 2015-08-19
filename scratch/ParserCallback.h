#pragma once

typedef enum {
    StatementResolution,
    StatementTitle,
} Statement;

typedef struct _ParseLocation {
    int line;
    int column;
} ParseLocation;

typedef void (*ParserCallback)(void *context, ParseLocation *location, Statement statement, ...);
typedef void (*ParserErrorCallback)(void *context, ParseLocation *location, const char *message);

static inline const char *Statement2String(Statement statement)
{
#define CASE(statement) case statement: return &(#statement[9])
    switch (statement) {
    CASE(StatementResolution);
    CASE(StatementTitle);

    default:
       break;
    }

    return "Unknown statement";
#undef CASE
}
