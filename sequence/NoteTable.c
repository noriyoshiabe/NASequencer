#include "NoteTable.h"

KeySign NoteTableGetKeySign(char keyChar, bool sharp, bool flat, bool major)
{
#define Index(c) (99 > c ? c - 92 : c - 99)

    const KeySign table[7][3][2] = {
        { // c
            {KeySignCMajor,      KeySignCMinor},
            {KeySignInvalid,     KeySignCSharpMinor},
            {KeySignInvalid,     KeySignInvalid},
        },
        { // d
            {KeySignDMajor,      KeySignDMinor},
            {KeySignInvalid,     KeySignDSharpMinor},
            {KeySignDFlatMajor,  KeySignInvalid},
        },
        { // e
            {KeySignEMajor,      KeySignEMinor},
            {KeySignInvalid,     KeySignInvalid},
            {KeySignEFlatMajor,  KeySignEFlatMinor},
        },
        { // f
            {KeySignFMajor,      KeySignFMinor},
            {KeySignFSharpMajor, KeySignFSharpMinor},
            {KeySignInvalid,     KeySignInvalid},
        },
        { // g
            {KeySignGMajor,      KeySignGMinor},
            {KeySignInvalid,     KeySignGSharpMinor},
            {KeySignGFlatMajor,  KeySignInvalid},
        },
        { // a
            {KeySignAMajor,      KeySignAMinor},
            {KeySignInvalid,     KeySignInvalid},
            {KeySignAFlatMajor,  KeySignInvalid},
        },
        { // b
            {KeySignBMajor,      KeySignBMinor},
            {KeySignInvalid,     KeySignInvalid},
            {KeySignBFlatMajor,  KeySignBFlatMinor},
        },
    };

    int sharpFlatIdx = sharp ? 1 : flat ? 2 : 0;
    int majorMinorIdx = major ? 0 : 1;

    return table[Index(keyChar)][sharpFlatIdx][majorMinorIdx];
}

int NoteTableGetNoteNo(KeySign keySign, BaseNote baseNote, Accidental accidental, int octave)
{
    const int noteTable[KeySignSize][BaseNoteSize][AccidentalSize] = {
        // CMajor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 5, 6, 3, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // GMajor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // DMajor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // AMajor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 8, 9, 7, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // EMajor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 3, 4, 2, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 8, 9, 7, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // BMajor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 3, 4, 2, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 8, 9, 7, 7}, // G
            {10,11, 9, 9}, // A
            {11,12,10,11}, // B
        },

        // FMajor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 5, 6, 4, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {10,11, 9,11}, // B
        },
        // BFlatMajor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 4, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {10,11, 9,11}, // B
        },
        // EFlatMajor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 4, 5}, // F
            { 7, 8, 6, 7}, // G
            { 8, 9, 7, 9}, // A
            {10,11, 9,11}, // B
        },
        // AFlatMajor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 1, 2, 0, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 4, 5}, // F
            { 7, 8, 6, 7}, // G
            { 8, 9, 7, 9}, // A
            {10,11, 9,11}, // B
        },
        // DFlatMajor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 1, 2, 0, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 4, 5}, // F
            { 6, 7, 5, 7}, // G
            { 8, 9, 7, 9}, // A
            {10,11, 9,11}, // B
        },
        // GFlatMajor
        { //  -  #  b  n
            {-1, 0,-2, 0}, // C
            { 1, 2, 0, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 4, 5}, // F
            { 6, 7, 5, 7}, // G
            { 8, 9, 7, 9}, // A
            {10,11, 9,11}, // B
        },

        // FSharpMajor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 3, 4, 2, 2}, // D
            { 5, 6, 4, 4}, // E
            { 6, 7, 5, 5}, // F
            { 8, 9, 7, 7}, // G
            {10,11, 9, 9}, // A
            {11,12,10,11}, // B
        },

        // AMinor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 5, 6, 3, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // EMinor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // BMinor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // FSharpMinor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 8, 9, 7, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // CSharpMinor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 3, 4, 2, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 8, 9, 7, 7}, // G
            { 9,10, 8, 9}, // A
            {11,12,10,11}, // B
        },
        // GSharpMinor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 3, 4, 2, 2}, // D
            { 4, 5, 3, 4}, // E
            { 6, 7, 5, 5}, // F
            { 8, 9, 7, 7}, // G
            {10,11, 9, 9}, // A
            {11,12,10,11}, // B
        },
        // DMinor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 4, 5, 3, 4}, // E
            { 5, 6, 3, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {10,11, 9,11}, // B
        },
        // GMinor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 3, 5}, // F
            { 7, 8, 6, 7}, // G
            { 9,10, 8, 9}, // A
            {10,11, 9,11}, // B
        },
        // CMinor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 2, 3, 1, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 3, 5}, // F
            { 7, 8, 6, 7}, // G
            { 8, 9, 7, 9}, // A
            {10,11, 9,11}, // B
        },
        // FMinor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 1, 2, 0, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 3, 5}, // F
            { 7, 8, 6, 7}, // G
            { 8, 9, 7, 9}, // A
            {10,11, 9,11}, // B
        },
        // BFlatMinor
        { //  -  #  b  n
            { 0, 1,-1, 0}, // C
            { 1, 2, 0, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 3, 5}, // F
            { 6, 7, 5, 7}, // G
            { 8, 9, 7, 9}, // A
            {10,11, 9,11}, // B
        },
        // EFlatMinor
        { //  -  #  b  n
            {-1, 0,-2, 0}, // C
            { 1, 2, 0, 2}, // D
            { 3, 4, 2, 4}, // E
            { 5, 6, 3, 5}, // F
            { 6, 7, 5, 7}, // G
            { 8, 9, 7, 9}, // A
            {10,11, 9,11}, // B
        },

        // DSharpMinor
        { //  -  #  b  n
            { 1, 2, 0, 0}, // C
            { 3, 4, 2, 2}, // D
            { 5, 6, 4, 4}, // E
            { 6, 7, 5, 5}, // F
            { 8, 9, 7, 7}, // G
            {10,11, 9, 9}, // A
            {11,12,10,11}, // B
        },
    };

    return noteTable[keySign][baseNote][accidental] + (octave + 2) * 12;
}

void KeySignGetMidiExpression(KeySign keySign, uint8_t *sf, uint8_t *mi)
{
    const uint8_t table[KeySignSize][2] = {
        {0x00, 0}, // KeySignCMajor
        {0x01, 0}, // KeySignGMajor
        {0x02, 0}, // KeySignDMajor
        {0x03, 0}, // KeySignAMajor
        {0x04, 0}, // KeySignEMajor
        {0x05, 0}, // KeySignBMajor
        {0xFF, 0}, // KeySignFMajor
        {0xFE, 0}, // KeySignBFlatMajor
        {0xFD, 0}, // KeySignEFlatMajor
        {0xFC, 0}, // KeySignAFlatMajor
        {0xFB, 0}, // KeySignDFlatMajor
        {0xFA, 0}, // KeySignGFlatMajor
        {0x06, 0}, // KeySignFSharpMajor
        {0x00, 1}, // KeySignAMinor
        {0x01, 1}, // KeySignEMinor
        {0x02, 1}, // KeySignBMinor
        {0x03, 1}, // KeySignFSharpMinor
        {0x04, 1}, // KeySignCSharpMinor
        {0x05, 1}, // KeySignGSharpMinor
        {0xFF, 1}, // KeySignDMinor
        {0xFE, 1}, // KeySignGMinor
        {0xFD, 1}, // KeySignCMinor
        {0xFC, 1}, // KeySignFMinor
        {0xFB, 1}, // KeySignBFlatMinor
        {0xFA, 1}, // KeySignEFlatMinor
        {0x06, 1}, // KeySignDSharpMinor
    };

    *sf = table[keySign][0];
    *mi = table[keySign][1];
}
