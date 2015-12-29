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
    int number;
    char *title;
    char *partSequence;
    NAMap *partMap;
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

typedef enum {
    RepeatStart,
    RepeatEnd,
    RepeatNth,
} SEMRepeatType;

typedef struct _SEMRepeat {
    Node node;
    SEMRepeatType type;
    int nth;
} SEMRepeat;

typedef struct _SEMBarLine {
    Node node;
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
    int instrument;
    int bank;
} SEMMidiVoice;

typedef struct _SEMPropagateAccidental {
    Node node;
    bool untilBar;
    bool allOctave;
} SEMPropagateAccidental;


typedef struct _SEMVisitor {
    void (*visitFile)(void *self, SEMFile *sem);
    void (*visitTune)(void *self, SEMTune *sem);
    void (*visitTempo)(void *self, SEMTempo *sem);
    void (*visitTime)(void *self, SEMTime *sem);
    void (*visitKey)(void *self, SEMKey *sem);
    void (*visitUnitNoteLength)(void *self, SEMUnitNoteLength *sem);
    void (*visitPart)(void *self, SEMPart *sem);
    void (*visitVoice)(void *self, SEMVoice *sem);
    void (*visitNote)(void *self, SEMNote *sem);
    void (*visitBrokenRhythm)(void *self, SEMBrokenRhythm *sem);
    void (*visitRest)(void *self, SEMRest *sem);
    void (*visitRepeat)(void *self, SEMRepeat *sem);
    void (*visitBarLine)(void *self, SEMBarLine *sem);
    void (*visitTie)(void *self, SEMTie *sem);
    void (*visitGraceNote)(void *self, SEMGraceNote *sem);
    void (*visitTuplet)(void *self, SEMTuplet *sem);
    void (*visitChord)(void *self, SEMChord *sem);
    void (*visitOverlay)(void *self, SEMOverlay *sem);
    void (*visitMidiVoice)(void *self, SEMMidiVoice *sem);
    void (*visitPropagateAccidental)(void *self, SEMPropagateAccidental *sem);
} SEMVisitor;


extern SEMFile *ABCSEMFileCreate(FileLocation *location);
extern SEMTune *ABCSEMTuneCreate(FileLocation *location);
extern SEMTempo *ABCSEMTempoCreate(FileLocation *location);
extern SEMTime *ABCSEMTimeCreate(FileLocation *location);
extern SEMKey *ABCSEMKeyCreate(FileLocation *location);
extern SEMUnitNoteLength *ABCSEMUnitNoteLengthCreate(FileLocation *location);
extern SEMPart *ABCSEMPartCreate(FileLocation *location);
extern SEMVoice *ABCSEMVoiceCreate(FileLocation *location);
extern SEMNote *ABCSEMNoteCreate(FileLocation *location);
extern SEMBrokenRhythm *ABCSEMBrokenRhythmCreate(FileLocation *location);
extern SEMRest *ABCSEMRestCreate(FileLocation *location);
extern SEMRepeat *ABCSEMRepeatCreate(FileLocation *location);
extern SEMBarLine *ABCSEMBarLineCreate(FileLocation *location);
extern SEMTie *ABCSEMTieCreate(FileLocation *location);
extern SEMGraceNote *ABCSEMGraceNoteCreate(FileLocation *location);
extern SEMTuplet *ABCSEMTupletCreate(FileLocation *location);
extern SEMChord *ABCSEMChordCreate(FileLocation *location);
extern SEMOverlay *ABCSEMOverlayCreate(FileLocation *location);
extern SEMMidiVoice *ABCSEMMidiVoiceCreate(FileLocation *location);
extern SEMPropagateAccidental *ABCSEMPropagateAccidentalCreate(FileLocation *location);


static inline const char *SEMRepeatType2String(SEMRepeatType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(RepeatStart);
    CASE(RepeatEnd);
    CASE(RepeatNth);
    }
#undef CASE
}
