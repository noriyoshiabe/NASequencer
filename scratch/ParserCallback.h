#pragma once

typedef void (*ParserCallback)(void *context, int statement, ...);
typedef void (*ParserErrorCallback)(void *context, int line, int column, const char *message);
