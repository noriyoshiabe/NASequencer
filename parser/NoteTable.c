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
    const int noteTable[KeySignSize][BaseNoteSize] = {
        {0, 2, 4, 5, 7, 9, 11}, // CMajor
        {0, 2, 4, 6, 7, 9, 11}, // GMajor
        {1, 2, 4, 6, 7, 9, 11}, // DMajor
        {1, 2, 4, 6, 8, 9, 11}, // AMajor
        {1, 3, 4, 6, 8, 9, 11}, // EMajor
        {1, 3, 4, 6, 8, 10, 11}, // BMajor

        {0, 2, 4, 5, 7, 9, 10}, // FMajor
        {0, 2, 3, 5, 7, 9, 10}, // BFlatMajor
        {0, 2, 3, 5, 7, 8, 10}, // EFlatMajor
        {0, 1, 3, 5, 7, 8, 10}, // AFlatMajor
        {0, 1, 3, 5, 6, 8, 10}, // DFlatMajor
        {-1, 1, 3, 5, 6, 8, 10}, // GFlatMajor

        {1, 3, 5, 6, 8, 10, 11}, // FSharpMajor

        {0, 2, 4, 5, 7, 9, 11}, // AMinor
        {0, 2, 4, 6, 7, 9, 11}, // EMinor
        {1, 2, 4, 6, 7, 9, 11}, // BMinor
        {1, 2, 4, 6, 8, 9, 11}, // FSharpMinor
        {1, 3, 4, 6, 8, 9, 11}, // CSharpMinor
        {1, 3, 4, 6, 8, 10, 11}, // GSharpMinor
        {0, 2, 4, 5, 7, 9, 10}, // DMinor
        {0, 2, 3, 5, 7, 9, 10}, // GMinor
        {0, 2, 3, 5, 7, 8, 10}, // CMinor
        {0, 1, 3, 5, 7, 8, 10}, // FMinor
        {0, 1, 3, 5, 6, 8, 10}, // BFlatMinor
        {-1, 1, 3, 5, 6, 8, 10}, // EFlatMinor

        {1, 3, 5, 6, 8, 10, 11}, // DSharpMinor
    };

    int noteNo = noteTable[AccidentalNatural == accidental ? KeySignCMajor : keySign][baseNote];
    switch (accidental) {
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
    default:
        break;
    }

    return noteNo + (octave + 2) * 12;
}

static const uint8_t KeySignMidiExpressionTable[KeySignSize][2] = {
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

void KeySignGetMidiExpression(KeySign keySign, uint8_t *sf, uint8_t *mi)
{
    *sf = KeySignMidiExpressionTable[keySign][0];
    *mi = KeySignMidiExpressionTable[keySign][1];
}

KeySign NoteTableGetKeySignByMidiExpression(uint8_t sf, uint8_t mi)
{
    for (int i = 0; i < sizeof(KeySignMidiExpressionTable)/ sizeof(KeySignMidiExpressionTable)[0]; ++i) {
        if (KeySignMidiExpressionTable[i][0] == sf && KeySignMidiExpressionTable[i][1] == mi) {
            return (KeySign)i;
        }
    }

    return KeySignInvalid;
}
