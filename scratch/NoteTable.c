#include "NoteTable.h"

#include <stdint.h>

typedef struct _NoteTableElement {
    int16_t baseNote;
    int16_t natural;
} NoteTableElement;

#define Index(c) (99 > c ? c - 92 : c - 99)

static NoteTableElement noteTable[NoteTableKeySignSize][7] = {
    // NoteTableKeySignCMajor
    {
        {12,  0}, // c
        {14,  0}, // d
        {16,  0}, // e
        {17,  0}, // f
        {19,  0}, // g
        {21,  0}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignGMajor
    {
        {12,  0}, // c
        {14,  0}, // d
        {16,  0}, // e
        {18, -1}, // f
        {19,  0}, // g
        {21,  0}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignDMajor
    {
        {13, -1}, // c
        {14,  0}, // d
        {16,  0}, // e
        {18, -1}, // f
        {19,  0}, // g
        {21,  0}, // a
        {23,  0}, // b
    },
    
    // NoteTableKeySignAMajor
    {
        {13, -1}, // c
        {14,  0}, // d
        {16,  0}, // e
        {18, -1}, // f
        {20, -1}, // g
        {21,  0}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignEMajor
    {
        {13, -1}, // c
        {15, -1}, // d
        {16,  0}, // e
        {18, -1}, // f
        {20, -1}, // g
        {21,  0}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignBMajor
    {
        {13, -1}, // c
        {15, -1}, // d
        {16,  0}, // e
        {18, -1}, // f
        {20, -1}, // g
        {22, -1}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignFMajor
    {
        {12,  0}, // c
        {14,  0}, // d
        {16,  0}, // e
        {17,  0}, // f
        {19,  0}, // g
        {21,  0}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignBFlatMajor
    {
        {12,  0}, // c
        {14,  0}, // d
        {15,  1}, // e
        {17,  0}, // f
        {19,  0}, // g
        {21,  0}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignEFlatMajor
    {
        {12,  0}, // c
        {14,  0}, // d
        {15,  1}, // e
        {17,  0}, // f
        {19,  0}, // g
        {20,  1}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignAFlatMajor
    {
        {12,  0}, // c
        {13,  1}, // d
        {15,  1}, // e
        {17,  0}, // f
        {19,  0}, // g
        {20,  1}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignDFlatMajor
    {
        {12,  0}, // c
        {13,  1}, // d
        {15,  1}, // e
        {17,  0}, // f
        {18,  1}, // g
        {20,  1}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignGFlatMajor
    {
        {11,  1}, // c
        {13,  1}, // d
        {15,  1}, // e
        {17,  0}, // f
        {18,  1}, // g
        {20,  1}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignFSharpMajor
    {
        {13, -1}, // c
        {15, -1}, // d
        {17, -1}, // e
        {18, -1}, // f
        {20, -1}, // g
        {22, -1}, // a
        {23,  0}, // b
    },


    // NoteTableKeySignAMinor
    {
        {12,  0}, // c
        {14,  0}, // d
        {16,  0}, // e
        {17,  0}, // f
        {19,  0}, // g
        {21,  0}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignEMinor
    {
        {12,  0}, // c
        {14,  0}, // d
        {16,  0}, // e
        {18, -1}, // f
        {19,  0}, // g
        {21,  0}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignBMinor
    {
        {13, -1}, // c
        {14,  0}, // d
        {16,  0}, // e
        {18, -1}, // f
        {19,  0}, // g
        {21,  0}, // a
        {23,  0}, // b
    },
    
    // NoteTableKeySignFSharpMinor
    {
        {13, -1}, // c
        {14,  0}, // d
        {16,  0}, // e
        {18, -1}, // f
        {20, -1}, // g
        {21,  0}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignCSharpMinor
    {
        {13, -1}, // c
        {15, -1}, // d
        {16,  0}, // e
        {18, -1}, // f
        {20, -1}, // g
        {21,  0}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignGSharpMinor
    {
        {13, -1}, // c
        {15, -1}, // d
        {16,  0}, // e
        {18, -1}, // f
        {20, -1}, // g
        {22, -1}, // a
        {23,  0}, // b
    },

    // NoteTableKeySignDMinor
    {
        {12,  0}, // c
        {14,  0}, // d
        {16,  0}, // e
        {17,  0}, // f
        {19,  0}, // g
        {21,  0}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignGMinor
    {
        {12,  0}, // c
        {14,  0}, // d
        {15,  1}, // e
        {17,  0}, // f
        {19,  0}, // g
        {21,  0}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignCMinor
    {
        {12,  0}, // c
        {14,  0}, // d
        {15,  1}, // e
        {17,  0}, // f
        {19,  0}, // g
        {20,  1}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignFMinor
    {
        {12,  0}, // c
        {13,  1}, // d
        {15,  1}, // e
        {17,  0}, // f
        {19,  0}, // g
        {20,  1}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignBFlatMinor
    {
        {12,  0}, // c
        {13,  1}, // d
        {15,  1}, // e
        {17,  0}, // f
        {18,  1}, // g
        {20,  1}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignEFlatMinor
    {
        {11,  1}, // c
        {13,  1}, // d
        {15,  1}, // e
        {17,  0}, // f
        {18,  1}, // g
        {20,  1}, // a
        {22,  1}, // b
    },

    // NoteTableKeySignDSharpMinor
    {
        {13, -1}, // c
        {15, -1}, // d
        {17, -1}, // e
        {18, -1}, // f
        {20, -1}, // g
        {22, -1}, // a
        {23,  0}, // b
    },
};

int NoteTableGetBaseNoteNo(NoteTableKeySign keySign, char noteChar)
{
    return noteTable[keySign][Index(noteChar)].baseNote;
}

int NoteTableGetNaturalDiff(NoteTableKeySign keySign, char noteChar)
{
    return noteTable[keySign][Index(noteChar)].natural;
}

NoteTableKeySign NoteTableGetKeySign(char keyChar, bool sharp, bool flat, bool major)
{
    const NoteTableKeySign table[7][3][2] = {
        { // c
            {NoteTableKeySignCMajor,      NoteTableKeySignCMinor},
            {NoteTableKeySignInvalid,     NoteTableKeySignCSharpMinor},
            {NoteTableKeySignInvalid,     NoteTableKeySignInvalid},
        },
        { // d
            {NoteTableKeySignDMajor,      NoteTableKeySignDMinor},
            {NoteTableKeySignInvalid,     NoteTableKeySignDSharpMinor},
            {NoteTableKeySignDFlatMajor,  NoteTableKeySignInvalid},
        },
        { // e
            {NoteTableKeySignEMajor,      NoteTableKeySignEMinor},
            {NoteTableKeySignInvalid,     NoteTableKeySignInvalid},
            {NoteTableKeySignEFlatMajor,  NoteTableKeySignEFlatMinor},
        },
        { // f
            {NoteTableKeySignFMajor,      NoteTableKeySignFMinor},
            {NoteTableKeySignFSharpMajor, NoteTableKeySignFSharpMinor},
            {NoteTableKeySignInvalid,     NoteTableKeySignInvalid},
        },
        { // g
            {NoteTableKeySignGMajor,      NoteTableKeySignGMinor},
            {NoteTableKeySignInvalid,     NoteTableKeySignGSharpMinor},
            {NoteTableKeySignGFlatMajor,  NoteTableKeySignInvalid},
        },
        { // a
            {NoteTableKeySignAMajor,      NoteTableKeySignAMinor},
            {NoteTableKeySignInvalid,     NoteTableKeySignInvalid},
            {NoteTableKeySignAFlatMajor,  NoteTableKeySignInvalid},
        },
        { // b
            {NoteTableKeySignBMajor,      NoteTableKeySignBMinor},
            {NoteTableKeySignInvalid,     NoteTableKeySignInvalid},
            {NoteTableKeySignBFlatMajor,  NoteTableKeySignBFlatMinor},
        },
    };

    int sharpFlatIdx = sharp ? 1 : flat ? 2 : 0;
    int majorMinorIdx = major ? 0 : 1;

    return table[Index(keyChar)][sharpFlatIdx][majorMinorIdx];
}
