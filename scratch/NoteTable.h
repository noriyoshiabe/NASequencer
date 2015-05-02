#pragma once

#include <stdbool.h>

typedef enum {
    NoteTableKeySignInvalid = -1,

    NoteTableKeySignCMajor,
    NoteTableKeySignGMajor,
    /* // TODO
    NoteTableKeySignDMajor,
    NoteTableKeySignAMajor,
    NoteTableKeySignEMajor,
    NoteTableKeySignBMajor,

    NoteTableKeySignFMajor,
    NoteTableKeySignBFlatMajor,
    NoteTableKeySignEFlatMajor,
    NoteTableKeySignAFlatMajor,
    NoteTableKeySignDFlatMajor,
    NoteTableKeySignGFlatMajor,

    NoteTableKeySignFSharpMajor,

    NoteTableKeySignAMinor,
    NoteTableKeySignEMinor,
    NoteTableKeySignBMinor,
    NoteTableKeySignFSharpMinor,
    NoteTableKeySignCSharpMinor,
    NoteTableKeySignGSharpMinor,
    NoteTableKeySignDMinor,
    NoteTableKeySignGMinor,
    NoteTableKeySignCMinor,
    NoteTableKeySignFMinor,
    NoteTableKeySignBFlatMinor,
    NoteTableKeySignEFlatMinor,

    NoteTableKeySignDSharpMinor,
    */

    NoteTableKeySignSize
} NoteTableKeySign;

extern int NoteTableGetBaseNoteNo(NoteTableKeySign keySign, char noteChar);
extern int NoteTableGetNaturalDiff(NoteTableKeySign keySign, char noteChar);
NoteTableKeySign NoteTableGetKeySign(char keyChar, bool sharp, bool flat, bool major);
