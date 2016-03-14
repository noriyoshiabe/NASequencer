#pragma once

#include "Node.h"
#include "NoteTable.h"

typedef struct _SEMList {
    Node node;
} SEMList;

typedef struct _SEMTimebase {
    Node node;
    int timebase;
} SEMTimebase;

typedef struct _SEMTitle {
    Node node;
    char *title;
} SEMTitle;

typedef struct _SEMCopyright {
    Node node;
    char *text;
} SEMCopyright;

typedef struct _SEMMarker {
    Node node;
    char *text;
} SEMMarker;

typedef struct _SEMVelocityReverse {
    Node node;
} SEMVelocityReverse;

typedef struct _SEMOctaveReverse {
    Node node;
} SEMOctaveReverse;

typedef struct _SEMChannel {
    Node node;
    int number;
} SEMChannel;

typedef struct _SEMSynth {
    Node node;
    char *name;
} SEMSynth;

typedef struct _SEMBankSelect {
    Node node;
    int bankNo;
} SEMBankSelect;

typedef struct _SEMProgramChange {
    Node node;
    int programNo;
} SEMProgramChange;

typedef struct _SEMVolume {
    Node node;
    int value;
} SEMVolume;

typedef struct _SEMChorus {
    Node node;
    int value;
} SEMChorus;

typedef struct _SEMReverb {
    Node node;
    int value;
} SEMReverb;

typedef struct _SEMExpression {
    Node node;
    int value;
} SEMExpression;

typedef struct _SEMPan {
    Node node;
    int value;
} SEMPan;

typedef struct _SEMDetune {
    Node node;
    int value;
} SEMDetune;

typedef struct _SEMTempo {
    Node node;
    float tempo;
} SEMTempo;

typedef struct _NoteLength {
    int length;
    int dotCount;
    int step;
} NoteLength;

typedef struct _SEMNote {
    Node node;
    BaseNote baseNote;
    Accidental accidental;
    NoteLength length;
    char *noteString;
} SEMNote;

typedef struct _SEMRest {
    Node node;
    NoteLength length;
} SEMRest;

typedef struct _SEMOctave {
    Node node;
    char direction;
    int value;
} SEMOctave;

typedef struct _SEMTranspose {
    Node node;
    bool relative;
    int value;
} SEMTranspose;

typedef struct _SEMTie {
    Node node;
} SEMTie;

typedef struct _SEMLength {
    Node node;
    int length;
} SEMLength;

typedef struct _SEMGatetime {
    Node node;
    bool absolute;
    int value;
} SEMGatetime;

typedef struct _SEMVelocity {
    Node node;
    char direction;
    bool absolute;
    int value;
} SEMVelocity;

typedef struct _SEMTuplet {
    Node node;
    int division;
    NoteLength length;
} SEMTuplet;

typedef struct _SEMTrackChange {
    Node node;
} SEMTrackChange;

typedef struct _SEMRepeat {
    Node node;
    int times;
} SEMRepeat;

typedef struct _SEMRepeatBreak {
    Node node;
} SEMRepeatBreak;

typedef struct _SEMChord {
    Node node;
} SEMChord;

typedef struct _SEMVisitor {
    void (*visitList)(void *self, SEMList *sem);
    void (*visitTimebase)(void *self, SEMTimebase *sem);
    void (*visitTitle)(void *self, SEMTitle *sem);
    void (*visitCopyright)(void *self, SEMCopyright *sem);
    void (*visitMarker)(void *self, SEMMarker *sem);
    void (*visitVelocityReverse)(void *self, SEMVelocityReverse *sem);
    void (*visitOctaveReverse)(void *self, SEMOctaveReverse *sem);
    void (*visitChannel)(void *self, SEMChannel *sem);
    void (*visitSynth)(void *self, SEMSynth *sem);
    void (*visitBankSelect)(void *self, SEMBankSelect *sem);
    void (*visitProgramChange)(void *self, SEMProgramChange *sem);
    void (*visitVolume)(void *self, SEMVolume *sem);
    void (*visitChorus)(void *self, SEMChorus *sem);
    void (*visitReverb)(void *self, SEMReverb *sem);
    void (*visitExpression)(void *self, SEMExpression *sem);
    void (*visitPan)(void *self, SEMPan *sem);
    void (*visitDetune)(void *self, SEMDetune *sem);
    void (*visitTempo)(void *self, SEMTempo *sem);
    void (*visitNote)(void *self, SEMNote *sem);
    void (*visitRest)(void *self, SEMRest *sem);
    void (*visitOctave)(void *self, SEMOctave *sem);
    void (*visitTransepose)(void *self, SEMTranspose *sem);
    void (*visitTie)(void *self, SEMTie *sem);
    void (*visitLength)(void *self, SEMLength *sem);
    void (*visitGatetime)(void *self, SEMGatetime *sem);
    void (*visitVelocity)(void *self, SEMVelocity *sem);
    void (*visitTuplet)(void *self, SEMTuplet *sem);
    void (*visitTrackChange)(void *self, SEMTrackChange *sem);
    void (*visitRepeat)(void *self, SEMRepeat *sem);
    void (*visitRepeatBreak)(void *self, SEMRepeatBreak *sem);
    void (*visitChord)(void *self, SEMChord *sem);
} SEMVisitor;

extern SEMList *MMLSEMListCreate(FileLocation *location);
extern SEMTimebase *MMLSEMTimebaseCreate(FileLocation *location);
extern SEMTitle *MMLSEMTitleCreate(FileLocation *location);
extern SEMCopyright *MMLSEMCopyrightCreate(FileLocation *location);
extern SEMMarker *MMLSEMMarkerCreate(FileLocation *location);
extern SEMVelocityReverse *MMLSEMVelocityReverseCreate(FileLocation *location);
extern SEMOctaveReverse *MMLSEMOctaveReverseCreate(FileLocation *location);
extern SEMChannel *MMLSEMChannelCreate(FileLocation *location);
extern SEMSynth *MMLSEMSynthCreate(FileLocation *location);
extern SEMBankSelect *MMLSEMBankSelectCreate(FileLocation *location);
extern SEMProgramChange *MMLSEMProgramChangeCreate(FileLocation *location);
extern SEMVolume *MMLSEMVolumeCreate(FileLocation *location);
extern SEMChorus *MMLSEMChorusCreate(FileLocation *location);
extern SEMReverb *MMLSEMReverbCreate(FileLocation *location);
extern SEMExpression *MMLSEMExpressionCreate(FileLocation *location);
extern SEMPan *MMLSEMPanCreate(FileLocation *location);
extern SEMDetune *MMLSEMDetuneCreate(FileLocation *location);
extern SEMTempo *MMLSEMTempoCreate(FileLocation *location);
extern SEMNote *MMLSEMNoteCreate(FileLocation *location);
extern SEMRest *MMLSEMRestCreate(FileLocation *location);
extern SEMOctave *MMLSEMOctaveCreate(FileLocation *location);
extern SEMTranspose *MMLSEMTransposeCreate(FileLocation *location);
extern SEMTie *MMLSEMTieCreate(FileLocation *location);
extern SEMLength *MMLSEMLengthCreate(FileLocation *location);
extern SEMGatetime *MMLSEMGatetimeCreate(FileLocation *location);
extern SEMVelocity *MMLSEMVelocityCreate(FileLocation *location);
extern SEMTuplet *MMLSEMTupletCreate(FileLocation *location);
extern SEMTrackChange *MMLSEMTrackChangeCreate(FileLocation *location);
extern SEMRepeat *MMLSEMRepeatCreate(FileLocation *location);
extern SEMRepeatBreak *MMLSEMRepeatBreakCreate(FileLocation *location);
extern SEMChord *MMLSEMChordCreate(FileLocation *location);
