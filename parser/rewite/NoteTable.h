#pragma once

#include <stdint.h>
#include <stdbool.h>

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
    AccidentalDoubleSharp,
    AccidentalFlat,
    AccidentalDoubleFlat,
    AccidentalNatural,
} Accidental;

typedef enum {
    ModeMajor,
    ModeMinor,

    ModeIonian,
    ModeAeolian,
    ModeMixolydian,
    ModeDorian,
    ModePhrygian,
    ModeLydian,
    ModeLocrian,

    ModeExplicit,
} Mode;

typedef struct _MidiKeySign {
    int8_t sf;
    int8_t mi;
} MidiKeySign;

typedef struct _NoteTable NoteTable;

extern NoteTable *NoteTableCreate(BaseNote baseNote, bool sharp, bool flat, Mode mode);
extern NoteTable *NoteTableRetain(NoteTable *self);
extern void NoteTableRelease(NoteTable *self);
extern bool NoteTableHasUnusualKeySign(NoteTable *self);
extern void NoteTableAppendAccidental(NoteTable *self, BaseNote baseNote, Accidental accidental);
extern MidiKeySign NoteTableGetMidiKeySign(NoteTable *self);
extern int NoteTableGetNoteNo(NoteTable *self, BaseNote baseNote, Accidental accidental, int octave);
extern const char *MidiKeySign2String(MidiKeySign keySign);
