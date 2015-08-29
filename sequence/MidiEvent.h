#pragma once

#include <stdbool.h>

typedef enum {
    KeySignInvalid = -1,

    KeySignCMajor,
    KeySignGMajor,
    KeySignDMajor,
    KeySignAMajor,
    KeySignEMajor,
    KeySignBMajor,

    KeySignFMajor,
    KeySignBFlatMajor,
    KeySignEFlatMajor,
    KeySignAFlatMajor,
    KeySignDFlatMajor,
    KeySignGFlatMajor,

    KeySignFSharpMajor,

    KeySignAMinor,
    KeySignEMinor,
    KeySignBMinor,
    KeySignFSharpMinor,
    KeySignCSharpMinor,
    KeySignGSharpMinor,
    KeySignDMinor,
    KeySignGMinor,
    KeySignCMinor,
    KeySignFMinor,
    KeySignBFlatMinor,
    KeySignEFlatMinor,

    KeySignDSharpMinor,

    KeySignSize
} KeySign;

typedef enum {
    BaseNote_C,
    BaseNote_D,
    BaseNote_E,
    BaseNote_F,
    BaseNote_G,
    BaseNote_A,
    BaseNote_B,
} BaseNote;

typedef enum {
    AccidentalNone,
    AccidentalSharp,
    AccidentalFlat,
    AccidentalNatural,
} Accidental;

typedef enum {
    MidiEventTypeNote,
    MidiEventTypeTempo,
    MidiEventTypeTime,
    MidiEventTypeKey,
    MidiEventTypeMarker,
    MidiEventTypeVoice,
    MidiEventTypeVolume,
    MidiEventTypePan,
    MidiEventTypeChorus,
    MidiEventTypeReverb,
} MidiEventType;

typedef struct _MidiEvent {
    MidiEventType type;
    int tick;
} MidiEvent;

typedef struct _NoteEvent {
    MidiEventType type;
    int tick;
    int channel;
    int noteNo;
    int gatetime;
    int velocity;
} NoteEvent;

typedef struct _TempoEvent {
    MidiEventType type;
    int tick;
    float tempo;
} TempoEvent;

typedef struct _TimeEvent {
    MidiEventType type;
    int tick;
    int numerator;
    int denominator;
} TimeEvent;

typedef struct _KeyEvent {
    MidiEventType type;
    int tick;
    KeySign keySign;
} KeyEvent;

typedef struct _MarkerEvent {
    MidiEventType type;
    int tick;
    int length;
    char text[];
} MarkerEvent;

typedef struct _VoiceEvent {
    MidiEventType type;
    int tick;
    int channel;
    int msb;
    int lsb;
    int programNo;
} VoiceEvent;

typedef struct _VolumeEvent {
    MidiEventType type;
    int tick;
    int channel;
    int value;
} VolumeEvent;

typedef struct _PanEvent {
    MidiEventType type;
    int tick;
    int channel;
    int value;
} PanEvent;

typedef struct _ChorusEvent {
    MidiEventType type;
    int tick;
    int channel;
    int value;
} ChorusEvent;

typedef struct _ReverbEvent {
    MidiEventType type;
    int tick;
    int channel;
    int value;
} ReverbEvent;

extern void *MidiEventAlloc(MidiEventType type, int tick, int extraSize);

extern KeySign KeySignTableGetKeySign(char keyChar, bool sharp, bool flat, bool major);

static inline char *KeySign2String(KeySign keySign)
{
#define CASE(keySign) case keySign: return &(#keySign[7])
    switch (keySign) {
    CASE(KeySignCMajor);
    CASE(KeySignGMajor);
    CASE(KeySignDMajor);
    CASE(KeySignAMajor);
    CASE(KeySignEMajor);
    CASE(KeySignBMajor);

    CASE(KeySignFMajor);
    CASE(KeySignBFlatMajor);
    CASE(KeySignEFlatMajor);
    CASE(KeySignAFlatMajor);
    CASE(KeySignDFlatMajor);
    CASE(KeySignGFlatMajor);

    CASE(KeySignFSharpMajor);

    CASE(KeySignAMinor);
    CASE(KeySignEMinor);
    CASE(KeySignBMinor);
    CASE(KeySignFSharpMinor);
    CASE(KeySignCSharpMinor);
    CASE(KeySignGSharpMinor);
    CASE(KeySignDMinor);
    CASE(KeySignGMinor);
    CASE(KeySignCMinor);
    CASE(KeySignFMinor);
    CASE(KeySignBFlatMinor);
    CASE(KeySignEFlatMinor);

    CASE(KeySignDSharpMinor);

    default:
       break;
    }

    return "Unknown key sign";
#undef CASE
}
