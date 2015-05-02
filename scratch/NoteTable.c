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
            {NoteTableKeySignCMajor,  NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
        },
        { // d
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
        },
        { // e
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
        },
        { // f
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
        },
        { // g
            {NoteTableKeySignGMajor,  NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
        },
        { // a
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
        },
        { // b
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
            {NoteTableKeySignInvalid, NoteTableKeySignInvalid},
        },
    };

    int sharpFlatIdx = sharp ? 1 : flat ? 2 : 0;
    int majorMinorIdx = major ? 0 : 1;

    return table[Index(keyChar)][sharpFlatIdx][majorMinorIdx];
}
