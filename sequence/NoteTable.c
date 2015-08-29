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
