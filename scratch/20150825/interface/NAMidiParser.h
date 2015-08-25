#pragma once

#include <stdbool.h>
#include <stdarg.h>

typedef enum {
} NAMidiParserEventType;

typedef enum {
} NAMidiParserErrorKind;

typedef struct _NAMidiParserError {
    NAMidiParserErrorKind kind;
    const char *filepath;
    int line;
    int column;
} NAMidiParserError;

typedef struct _NAMidiParser NAMidiParser;
typedef void (*NAMidiParserEventHandler)(void *receiver, NAMidiParserEventType type, va_list argList);

extern NAMidiParser *NAMidiParserCreate(NAMidiParserEventHandler handler, void *receiver);
extern void NAMidiParserDestroy(NAMidiParser *self);
extern bool NAMidiParserExecuteParse(NAMidiParser *self, FILE *input, NAMidiParserError *error);
extern const char *NAMidiParserErrorKind2String(NAMidiParserErrorKind kind);


typedef struct _NAMidiStatement NAMidiStatement;

typedef struct _TimeTableRecord {
    int tickStart;
    int tickEnd;
    int measureStart;
    int measureEnd;
    int64_t usecStart;
    int64_t usecEnd;
    float tempo;
    int16_t numerator;
    int16_t denominator;
} TimeTableRecord;

typedef struct _TimeTable {
    int resolution;
    NAArray *records;
} TimeTable;

typedef struct _NAMidiSequence {
    NAArray *events;
    TimeTable *timeTable;
} NAMidiSequence;

typedef struct _NAMidiSong {
    char *title;
    NAMidiSequence *sequence;
    NAArray *patterns;
    NAArray *observers;
} NAMidiSong;

typedef struct _NAMidiPattern {
    char *name;
    NAMidiSequence *sequence;
    NAArray *observers;
} NAMidiPattern;

typedef struct _NAMidiSongEditor {
    NAMidiSong *song;
} NAMidiSongEditor;

extern NAMidiSongEditor *NAMidiSongEditorCreate(NAMidiSong *song);
extern void NAMidiSongEditorDestroy(NAMidiSongEditor *self);
extern bool NAMidiSongEditorProcessEvent(NAMidiSongEditor *self, NAMidiSongEditorError *error, NAMidiParserEventType type, va_list argList);
extern void NAMidiSongEditorReset(NAMidiSongEditor *self);

typedef struct _NAMidiPlayer {
    NAMidiSequence *sequence;
};
