#pragma once

#include "Node.h"
#include "NoteTable.h"
#include "ABCAST.h"
#include "NAArray.h"
#include "NAMap.h"
#include "NASet.h"

typedef struct _SEMFile {
    Node node;
} SEMFile;

typedef struct _SEMTune {
    Node node;
    int number;
    NAArray *titleList;
    char *partSequence;
    NAMap *partMap;
    NAMap *voiceMap;
} SEMTune;

typedef struct _SEMKey {
    Node node;
    NoteTable *noteTable;
    int transpose;
    int octave;

    const ASTKeyParam *tonic;
    const ASTKeyParam *mode;
    NAArray *accidentals;
} SEMKey;

typedef struct _SEMMeter {
    Node node;
    int numerator;
    int denominator;
} SEMMeter;

typedef struct _SEMUnitNoteLength {
    Node node;
    int length;
} SEMUnitNoteLength;

typedef struct _SEMTempo {
    Node node;
    float tempo;

    NAArray *beatUnits;
    const ASTTempoParam *beatCount;
} SEMTempo;

typedef struct _SEMPart {
    Node node;
    char *identifier;
    NAMap *listMap;
} SEMPart;

typedef struct _SEMList {
    Node node;
    char *voiceId;
} SEMList;

typedef struct _SEMVoice {
    Node node;
    char *identifier;
    int transpose;
    int octave;
} SEMVoice;

typedef struct _NoteLength {
    int multiplier;
    int divider;
} NoteLength;

typedef enum {
    Accent,
    Emphasis,
    PianoPianissimo,
    Pianissimo,
    Piano,
    MezzoPiano,
    MezzoForte,
    Forte,
    Foruthisimo,
    ForteForuthisimo,
} SEMDecorationType;

typedef struct _SEMDecoration {
    Node node;
    SEMDecorationType type;
} SEMDecoration;

typedef struct _SEMNote {
    Node node;
    BaseNote baseNote;
    Accidental accidental;
    int octave;
    NoteLength length;
    char *noteString;
} SEMNote;

typedef struct _SEMBrokenRhythm {
    Node node;
    char direction;
} SEMBrokenRhythm;

typedef enum {
    RestUnitNote,
    RestMeasure,
} SEMRestType;

typedef struct _SEMRest {
    Node node;
    SEMRestType type;
    NoteLength length;
} SEMRest;

typedef enum {
    RepeatStart,
    RepeatEnd,
    RepeatNth,
} SEMRepeatType;

typedef struct _SEMRepeat {
    Node node;
    SEMRepeatType type;
    NASet *nthSet;
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
    int division;
    int time;
    int count;
} SEMTuplet;

typedef struct _SEMChord {
    Node node;
    NoteLength length;
} SEMChord;

typedef struct _SEMOverlay {
    Node node;
} SEMOverlay;

typedef struct _SEMMidiVoice {
    Node node;
    char *voiceId;
    int instrument;
    int bank;
    bool mute;
} SEMMidiVoice;

typedef struct _SEMPropagateAccidental {
    Node node;
    bool untilBar;
    bool allOctave;
} SEMPropagateAccidental;


typedef struct _SEMVisitor {
    void (*visitFile)(void *self, SEMFile *sem);
    void (*visitTune)(void *self, SEMTune *sem);
    void (*visitKey)(void *self, SEMKey *sem);
    void (*visitMeter)(void *self, SEMMeter *sem);
    void (*visitUnitNoteLength)(void *self, SEMUnitNoteLength *sem);
    void (*visitTempo)(void *self, SEMTempo *sem);
    void (*visitPart)(void *self, SEMPart *sem);
    void (*visitList)(void *self, SEMList *sem);
    void (*visitVoice)(void *self, SEMVoice *sem);
    void (*visitDecoration)(void *self, SEMDecoration *sem);
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
extern SEMKey *ABCSEMKeyCreate(FileLocation *location);
extern SEMMeter *ABCSEMMeterCreate(FileLocation *location);
extern SEMUnitNoteLength *ABCSEMUnitNoteLengthCreate(FileLocation *location);
extern SEMTempo *ABCSEMTempoCreate(FileLocation *location);
extern SEMPart *ABCSEMPartCreate(FileLocation *location);
extern SEMList *ABCSEMListCreate(FileLocation *location);
extern SEMVoice *ABCSEMVoiceCreate(FileLocation *location);
extern SEMDecoration *ABCSEMDecorationCreate(FileLocation *location);
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

static inline const char *SEMRestType2String(SEMRestType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(RestUnitNote);
    CASE(RestMeasure);
    }
#undef CASE
}

static inline const char *SEMDecorationType2String(SEMDecorationType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(Accent);
    CASE(Emphasis);
    CASE(PianoPianissimo);
    CASE(Pianissimo);
    CASE(Piano);
    CASE(MezzoPiano);
    CASE(MezzoForte);
    CASE(Forte);
    CASE(Foruthisimo);
    CASE(ForteForuthisimo);
    }
#undef CASE
}
