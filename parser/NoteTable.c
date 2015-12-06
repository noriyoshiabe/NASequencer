#include "NoteTable.h"

#include <stdio.h>
#include <stdlib.h>

struct _NoteTable {
    int refCount;
    MidiKeySign midiKeySign;
    int baseNote2Sf[7];
    bool unusualKey;
};

NoteTable *NoteTableCreate(BaseNote baseNote, bool sharp, bool flat, Mode mode)
{
    NoteTable *self = calloc(1, sizeof(NoteTable));
    ++self->refCount;

    const int noteSf[] = {
        0,  // C
        2,  // D
        4,  // E
        -1, // F
        1,  // G
        3,  // A
        5   // B
    };

    int sf = noteSf[baseNote];
    if (sharp) {
        sf += 7;
    }
    else if (flat) {
        sf -= 7;
    }

    const int modeShift[] = {
        0,  // Major
        -3, // Minor
        0,  // Ionian
        -3, // Aeolian
        -1, // Mixolydian
        -2, // Dorian
        -4, // Phrygian
        1,  // Lydian
        -5, // Locrian
        0,  // Explicit
    };

    sf += modeShift[mode];

    self->unusualKey = sf < -7 || 7 < sf;
    self->midiKeySign.sf = -7 > sf ? sf + 12 : 7 < sf ? sf - 12 : sf;
    self->midiKeySign.mi = ModeMinor == mode ? 1 : 0;

    if (0 < self->midiKeySign.sf) {
        int sharpCount = self->midiKeySign.sf;
        for (int i = 0; i < sharpCount; ++i) {
            switch (i + 1) {
            case 1: self->baseNote2Sf[BaseNote_F] = 1; break;
            case 2: self->baseNote2Sf[BaseNote_C] = 1; break;
            case 3: self->baseNote2Sf[BaseNote_G] = 1; break;
            case 4: self->baseNote2Sf[BaseNote_D] = 1; break;
            case 5: self->baseNote2Sf[BaseNote_A] = 1; break;
            case 6: self->baseNote2Sf[BaseNote_E] = 1; break;
            case 7: self->baseNote2Sf[BaseNote_B] = 1; break;
            default: break;
            }
        }
    }
    else if (0 > self->midiKeySign.sf) {
        int flatCount = -self->midiKeySign.sf;
        for (int i = 0; i < flatCount; ++i) {
            switch (i + 1) {
            case 1: self->baseNote2Sf[BaseNote_B] = -1; break;
            case 2: self->baseNote2Sf[BaseNote_E] = -1; break;
            case 3: self->baseNote2Sf[BaseNote_A] = -1; break;
            case 4: self->baseNote2Sf[BaseNote_D] = -1; break;
            case 5: self->baseNote2Sf[BaseNote_G] = -1; break;
            case 6: self->baseNote2Sf[BaseNote_C] = -1; break;
            case 7: self->baseNote2Sf[BaseNote_F] = -1; break;
            default: break;
            }
        }
    }

    return self;
}

NoteTable *NoteTableRetain(NoteTable *self)
{
    ++self->refCount;
    return self;
}

void NoteTableRelease(NoteTable *self)
{
    if (0 == --self->refCount) {
        free(self);
    }
}

bool NoteTableHasUnusualKeySign(NoteTable *self)
{
    return self->unusualKey;
}

void NoteTableAppendAccidental(NoteTable *self, BaseNote baseNote, Accidental accidental)
{
    switch (accidental) {
    case AccidentalSharp:
        self->baseNote2Sf[baseNote] = 1;
        break;
    case AccidentalDoubleSharp:
        self->baseNote2Sf[baseNote] = 2;
        break;
    case AccidentalFlat:
        self->baseNote2Sf[baseNote] = -1;
        break;
    case AccidentalDoubleFlat:
        self->baseNote2Sf[baseNote] = -2;
        break;
    case AccidentalNatural:
        self->baseNote2Sf[baseNote] = 0;
        break;
    default:
        break;
    }
}

MidiKeySign NoteTableGetMidiKeySign(NoteTable *self)
{
    return self->midiKeySign;
}

int NoteTableGetNoteNo(NoteTable *self, BaseNote baseNote, Accidental accidental, int octave)
{
    const int naturalNoteNo[] = {
        0, // C
        2, // D
        4, // E
        5, // F
        7, // G
        9, // A
        11 // B
    };

    int noteNo = naturalNoteNo[baseNote];

    switch (accidental) {
    case AccidentalNone:
        noteNo += self->baseNote2Sf[baseNote];
        break;
    case AccidentalSharp:
        noteNo += 1;
        break;
    case AccidentalDoubleSharp:
        noteNo += 2;
        break;
    case AccidentalFlat:
        noteNo -= 1;
        break;
    case AccidentalDoubleFlat:
        noteNo -= 2;
        break;
    case AccidentalNatural:
        break;
    }

    return noteNo + (octave + 2) * 12;
}

const char *MidiKeySign2String(MidiKeySign keySign)
{
    struct {
        int8_t sf;
        int8_t mi;
        const char *name;
    } keySignTable[] = {
        {0x00, 0, "C Major"},
        {0x01, 0, "G Major"},
        {0x02, 0, "D Major"},
        {0x03, 0, "A Major"},
        {0x04, 0, "E Major"},
        {0x05, 0, "B Major"},
        {0x06, 0, "F# Major"},
        {0x07, 0, "C# Major"},
        {0xFF, 0, "F Major"},
        {0xFE, 0, "Bb Major"},
        {0xFD, 0, "Eb Major"},
        {0xFC, 0, "Ab Major"},
        {0xFB, 0, "Db Major"},
        {0xFA, 0, "Gb Major"},
        {0xF0, 0, "Cb Major"},
        {0x00, 1, "A Minor"},
        {0x01, 1, "E Minor"},
        {0x02, 1, "B Minor"},
        {0x03, 1, "F# Minor"},
        {0x04, 1, "C# Minor"},
        {0x05, 1, "G# Minor"},
        {0x06, 1, "D# Minor"},
        {0x07, 1, "A# Minor"},
        {0xFF, 1, "D Minor"},
        {0xFE, 1, "G Minor"},
        {0xFD, 1, "C Minor"},
        {0xFC, 1, "F Minor"},
        {0xFB, 1, "Bb Minor"},
        {0xFA, 1, "Eb Minor"},
        {0xF0, 1, "Ab Minor"},
    };

    for (int i = 0; i < sizeof(keySignTable)/ sizeof(keySignTable)[0]; ++i) {
        if (keySignTable[i].sf == keySign.sf && keySignTable[i].mi == keySign.mi) {
            return keySignTable[i].name;
        }
    }

    return "Unknown key singature";
}

void NoteTableDump(NoteTable *self, int indent)
{
    printf("%*s[NoteTable] sf=%d mi=%d unusualKey=%s baseNote2Sf=%d,%d,%d,%d,%d,%d,%d\n", indent, "",
            self->midiKeySign.sf,
            self->midiKeySign.mi,
            self->unusualKey ? "true" : "false",
            self->baseNote2Sf[0],
            self->baseNote2Sf[1],
            self->baseNote2Sf[2],
            self->baseNote2Sf[3],
            self->baseNote2Sf[4],
            self->baseNote2Sf[5],
            self->baseNote2Sf[6]);
}
