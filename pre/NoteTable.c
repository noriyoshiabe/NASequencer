#include "NoteTable.h"

#include <stdlib.h>

struct _NoteTable {
    int refCount;
    MidiKeySign midiKeySign;
    int baseNote2Sf[7];
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
        int flatCount = self->midiKeySign.sf;
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
