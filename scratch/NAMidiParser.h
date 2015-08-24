#pragma once

#include <stdbool.h>
#include <stdarg.h>

typedef enum {
    NAMidiParserEventTypeNote,
    NAMidiParserEventTypeResolution,
    NAMidiParserEventTypeTempo,
    NAMidiParserEventTypeTimeSign,
    NAMidiParserEventTypeMeasure,
    NAMidiParserEventTypePattern,
    NAMidiParserEventTypePatternDefine,
    NAMidiParserEventTypeEnd,
    NAMidiParserEventTypeTrack,
    NAMidiParserEventTypeChannel,
    NAMidiParserEventTypeSound,
    NAMidiParserEventTypeMarker,
    NAMidiParserEventTypeTitle,
    NAMidiParserEventTypeVolume,
    NAMidiParserEventTypePan,
    NAMidiParserEventTypeChorus,
    NAMidiParserEventTypeReverb,
    NAMidiParserEventTypeTranspose,
    NAMidiParserEventTypeKeySign,
    NAMidiParserEventTypeRest,
    NAMidiParserEventTypeIncludeFile,
} NAMidiParserEventType;

typedef enum {
    NAMidiParserErrorKindFileNotFound,
    NAMidiParserErrorKindSyntaxError,

    NAMidiParserErrorKindInvalidResolution,
    NAMidiParserErrorKindInvalidTempo,
    NAMidiParserErrorKindInvalidTimeSign,
    NAMidiParserErrorKindInvalidMeasure,
    NAMidiParserErrorKindInvalidTrack,
    NAMidiParserErrorKindInvalidChannel,
    NAMidiParserErrorKindInvalidVoiceMSB,
    NAMidiParserErrorKindInvalidVoiceLSB,
    NAMidiParserErrorKindInvalidVoiceProgramNo,
    NAMidiParserErrorKindInvalidVolume,
    NAMidiParserErrorKindInvalidPan,
    NAMidiParserErrorKindInvalidChorus,
    NAMidiParserErrorKindInvalidReverb,
    NAMidiParserErrorKindInvalidTranspose,
    NAMidiParserErrorKindInvalidNote,
    NAMidiParserErrorKindInvalidStep,
    NAMidiParserErrorKindInvalidGatetime,
    NAMidiParserErrorKindInvalidVelocity,

    NAMidiParserErrorKindIllegalEnd,
    NAMidiParserErrorKindEndMissing,
} NAMidiParserErrorKind;

typedef struct _NAMidiParserLocation {
    const char *filepath;
    int line;
    int column;
} NAMidiParserLocation;

typedef struct _NAMidiParserEvent {
    NAMidiParserEventType type;
    NAMidiParserLocation location;
} NAMidiParserEvent;

typedef struct _NAMidiParserError {
    NAMidiParserErrorKind kind;
    NAMidiParserLocation location;
} NAMidiParserError;

typedef struct _NAMidiParser NAMidiParser;

typedef struct _NAMidiParserCallbacks {
    void (*onParseEvent)(void *receiver, NAMidiParserEvent *event, va_list argList);
    void (*onParseError)(void *receiver, NAMidiParserError *error);
} NAMidiParserCallbacks;

extern NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks, void *receiver);
extern void NAMidiParserDestroy(NAMidiParser *self);
extern bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath);
extern const char *NAMidiParserErrorKind2String(NAMidiParserErrorKind kind);

#define NAMidiParserMeasureMax 9999
