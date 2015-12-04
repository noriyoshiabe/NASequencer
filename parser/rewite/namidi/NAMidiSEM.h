#pragma once

#include "Node.h"
#include "NoteTable.h"
#include "NAArray.h"
#include "NAMap.h"

#define SEMNOTE_OCTAVE_NONE -99

typedef struct _SEMList {
    Node node;
    NAMap *patternMap;
    char *identifier;
} SEMList;

typedef struct _SEMResolution {
    Node node;
    int resolution;
} SEMResolution;

typedef struct _SEMTitle {
    Node node;
    char *title;
} SEMTitle;

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
} SEMKey;

typedef struct _SEMMarker {
    Node node;
    char *text;
} SEMMarker;

typedef struct _SEMChannel {
    Node node;
    int number;
} SEMChannel;

typedef struct _SEMVoice {
    Node node;
    int msb;
    int lsb;
    int programNo;
} SEMVoice;

typedef struct _SEMSynth {
    Node node;
    char *name;
} SEMSynth;

typedef struct _SEMVolume {
    Node node;
    int value;
} SEMVolume;

typedef struct _SEMPan {
    Node node;
    int value;
} SEMPan;

typedef struct _SEMChorus {
    Node node;
    int value;
} SEMChorus;

typedef struct _SEMReverb {
    Node node;
    int value;
} SEMReverb;

typedef struct _SEMTranspose {
    Node node;
    int value;
} SEMTranspose;

typedef struct _SEMRest {
    Node node;
    int step;
} SEMRest;

typedef struct _SEMNote {
    Node node;
    int step;
    BaseNote baseNote;
    Accidental accidental;
    int octave;
    int gatetime;
    int velocity;
    char *noteString;
} SEMNote;

typedef struct _SEMPattern {
    Node node;
    char *identifier;
    NAArray *ctxIdList;
} SEMPattern;

typedef struct _SEMContext {
    Node node;
    NAArray *ctxIdList;
    SEMList *list;
} SEMContext;

typedef struct _SEMVisitor {
    void (*visitList)(void *self, SEMList *sem);
    void (*visitResolution)(void *self, SEMResolution *sem);
    void (*visitTitle)(void *self, SEMTitle *sem);
    void (*visitTempo)(void *self, SEMTempo *sem);
    void (*visitTime)(void *self, SEMTime *sem);
    void (*visitKey)(void *self, SEMKey *sem);
    void (*visitMarker)(void *self, SEMMarker *sem);
    void (*visitChannel)(void *self, SEMChannel *sem);
    void (*visitVoice)(void *self, SEMVoice *sem);
    void (*visitSynth)(void *self, SEMSynth *sem);
    void (*visitVolume)(void *self, SEMVolume *sem);
    void (*visitPan)(void *self, SEMPan *sem);
    void (*visitChorus)(void *self, SEMChorus *sem);
    void (*visitReverb)(void *self, SEMReverb *sem);
    void (*visitTranspose)(void *self, SEMTranspose *sem);
    void (*visitRest)(void *self, SEMRest *sem);
    void (*visitNote)(void *self, SEMNote *sem);
    void (*visitPattern)(void *self, SEMPattern *sem);
    void (*visitContext)(void *self, SEMContext *sem);
} SEMVisitor;

extern SEMList *NAMidiSEMListCreate(FileLocation *location);
extern SEMResolution *NAMidiSEMResolutionCreate(FileLocation *location);
extern SEMTitle *NAMidiSEMTitleCreate(FileLocation *location);
extern SEMTempo *NAMidiSEMTempoCreate(FileLocation *location);
extern SEMTime *NAMidiSEMTimeCreate(FileLocation *location);
extern SEMKey *NAMidiSEMKeyCreate(FileLocation *location);
extern SEMMarker *NAMidiSEMMarkerCreate(FileLocation *location);
extern SEMChannel *NAMidiSEMChannelCreate(FileLocation *location);
extern SEMVoice *NAMidiSEMVoiceCreate(FileLocation *location);
extern SEMSynth *NAMidiSEMSynthCreate(FileLocation *location);
extern SEMVolume *NAMidiSEMVolumeCreate(FileLocation *location);
extern SEMPan *NAMidiSEMPanCreate(FileLocation *location);
extern SEMChorus *NAMidiSEMChorusCreate(FileLocation *location);
extern SEMReverb *NAMidiSEMReverbCreate(FileLocation *location);
extern SEMTranspose *NAMidiSEMTransposeCreate(FileLocation *location);
extern SEMRest *NAMidiSEMRestCreate(FileLocation *location);
extern SEMNote *NAMidiSEMNoteCreate(FileLocation *location);
extern SEMPattern *NAMidiSEMPatternCreate(FileLocation *location);
extern SEMContext *NAMidiSEMContextCreate(FileLocation *location);
