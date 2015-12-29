#pragma once

#include "Node.h"
#include "NoteTable.h"
#include "NAArray.h"
#include "NAMap.h"

typedef struct _SEMFile {
    Node node;
} SEMFile;

typedef struct _SEMTune {
    Node node;
    NAMap *partMap;
    char *partSequence;
    int number;
    char *title;
} SEMTune;

typedef struct _SEMTempo {
    Node node;
    float tempo;
} SEMTempo;

typedef struct _SEMTime {
    Node node;
    int numerator;
    int denominator;
} SEMTime;

typedef struct _SEMKey {
    Node node;
    NoteTable *noteTable;
    int transpose;
    int octave;
} SEMKey;

typedef struct _SEMUnitNoteLength {
    Node node;
    int length;
} SEMUnitNoteLength;

typedef struct _SEMPart {
    Node node;
    char *identifier;
} SEMPart;

typedef struct _SEMVoice {
    Node node;
    char *identifier;
    int transpose;
    int octave;
} SEMVoice;

typedef struct _SEMNote {
    Node node;
    BaseNote baseNote;
    Accidental accidental;
    int octave;
    struct {
        int multiplier;
        int divider;
    } length;
} SEMNote;

typedef struct _SEMBrokenRhythm {
    Node node;
    char direction;
} SEMBrokenRhythm;

typedef struct _SEMRest {
    Node node;
    struct {
        int multiplier;
        int divider;
    } length;
} SEMRest;

typedef struct _SEMRepeat {
    Node node;
    int type; // TODO
    int number;
} SEMRepeat;

typedef struct _SEMBarLine {
    Node node;
    int type; // TODO
} SEMBarLine;

typedef struct _SEMTie {
    Node node;
} SEMTie;

typedef struct _SEMGraceNote {
    Node node;
} SEMGraceNote;

typedef struct _SEMTuplet {
    Node node;
    int p;
    int q;
    int r;
} SEMTuplet;

typedef struct _SEMChord {
    Node node;
    struct {
        int multiplier;
        int divider;
    } length;
} SEMChord;

typedef struct _SEMOverlay {
    Node node;
} SEMOverlay;

typedef struct _SEMMidiVoice {
    Node node;
    int bank;
    int programNo;
} SEMMidiVoice;

typedef struct _SEMPropagateAccidental {
    Node node;
    bool untilBar;
    bool allOctave;
} SEMPropagateAccidental;
