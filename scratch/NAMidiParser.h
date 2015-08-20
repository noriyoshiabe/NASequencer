#pragma once

#include <stdbool.h>
#include <stdarg.h>

typedef enum {
    NAMidiParserEventTypeNote,
    NAMidiParserEventTypeTempo,
    NAMidiParserEventTypeTime,
    NAMidiParserEventTypeSound,
    NAMidiParserEventTypeMarker,
} NAMidiParserEventType;

typedef enum {
    NAMidiParserErrorKindFileNotFound,
    NAMidiParserErrorKindSyntaxError,
} NAMidiParserErrorKind;

typedef struct _NAMidiParserError {
    NAMidiParserErrorKind kind;
    char *message;
    const char *filepath;
    int line;
    int column;
} NAMidiParserError;

typedef struct _NAMidiParser NAMidiParser;
typedef void (*NAMidiParserRenderHandler)(void *receiver, va_list argList);

extern NAMidiParser *NAMidiParserCreate();
extern void NAMidiParserDestroy(NAMidiParser *self);
extern bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath);
extern void NAMidiParserRender(NAMidiParser *self, void *view, NAMidiParserRenderHandler handler);
extern const NAMidiParserError *NAMidiParserGetError(NAMidiParser *self);
extern const char **NAMidiParserGetFilepaths(NAMidiParser *self);
