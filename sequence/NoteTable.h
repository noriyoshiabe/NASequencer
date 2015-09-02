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

extern KeySign NoteTableGetKeySign(char keyChar, bool sharp, bool flat, bool major);
extern int NoteTableGetNoteNo(KeySign keySign, BaseNote baseNote, Accidental Accidental, int octave);

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
