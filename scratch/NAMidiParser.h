#pragma once

#include <stdbool.h>
#include <stdarg.h>

typedef enum {
    NAMidiParserEventTypeNote,
    NAMidiParserEventTypeTempo,
    NAMidiParserEventTypeTime,
    NAMidiParserEventTypeSound,
    NAMidiParserEventTypeMarker,
    NAMidiParserEventTypeTitle,
    NAMidiParserEventTypeVolume,
    NAMidiParserEventTypePan,
    NAMidiParserEventTypeChorus,
    NAMidiParserEventTypeReverb,
    NAMidiParserEventTypeKeySign,
} NAMidiParserEventType;

typedef enum {
    NAMidiParserErrorKindFileNotFound,
    NAMidiParserErrorKindSyntaxError,

    NAMidiParserErrorKindIllegalResolution,
    NAMidiParserErrorKindInvalidResolution,
    NAMidiParserErrorKindInvalidTempo,
    NAMidiParserErrorKindInvalidTimeSign,
    NAMidiParserErrorKindInvalidMeasure,
    NAMidiParserErrorKindIllegalPatternDefineInPattern,
    NAMidiParserErrorKindIllegalPatternDefineInTrack,
    NAMidiParserErrorKindIllegalEnd,
    NAMidiParserErrorKindIllegalTrackStartInTrack,
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
    NAMidiParserErrorKindInvalidKeySign,
    NAMidiParserErrorKindInvalidNote,
    NAMidiParserErrorKindInvalidStep,
    NAMidiParserErrorKindInvalidGatetime,
    NAMidiParserErrorKindInvalidVelocity,

    NAMidiParserErrorKindPatternEndMissing,
    NAMidiParserErrorKindTrackEndMissing,
} NAMidiParserErrorKind;

typedef struct _NAMidiParserError {
    NAMidiParserErrorKind kind;
    const char *filepath;
    int line;
    int column;
} NAMidiParserError;

typedef struct _NAMidiParser NAMidiParser;
typedef void (*NAMidiParserRenderHandler)(void *receiver, va_list argList);

extern NAMidiParser *NAMidiParserCreate(NAMidiParserRenderHandler handler);
extern void NAMidiParserDestroy(NAMidiParser *self);
extern bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath);
extern const NAMidiParserError *NAMidiParserGetError(NAMidiParser *self);
extern const char **NAMidiParserGetFilepaths(NAMidiParser *self);
extern const char *NAMidiParserErrorKind2String(NAMidiParserErrorKind kind);
