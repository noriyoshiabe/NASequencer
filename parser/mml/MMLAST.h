#pragma once

#include "Node.h"

typedef struct _ASTRoot {
    Node node;
} ASTRoot;

typedef struct _ASTTimebase {
    Node node;
    int timebase;
} ASTTimebase;

typedef struct _ASTTitle {
    Node node;
    char *title;
} ASTTitle;

typedef struct _ASTMarker {
    Node node;
    char *text;
} ASTMarker;

typedef struct _ASTVelocityReverse {
    Node node;
} ASTVelocityReverse;

typedef struct _ASTOctaveReverse {
    Node node;
} ASTOctaveReverse;

typedef struct _ASTChannel {
    Node node;
    int number;
} ASTChannel;

typedef struct _ASTSynth {
    Node node;
    char *name;
} ASTSynth;

typedef struct _ASTBankSelect {
    Node node;
    int msb;
    int lsb;
} ASTBankSelect;

typedef struct _ASTProgramChange {
    Node node;
    int programNo;
} ASTProgramChange;

typedef struct _ASTVolume {
    Node node;
    int value;
} ASTVolume;

typedef struct _ASTChorus {
    Node node;
    int value;
} ASTChorus;

typedef struct _ASTReverb {
    Node node;
    int value;
} ASTReverb;

typedef struct _ASTExpression {
    Node node;
    int value;
} ASTExpression;

typedef struct _ASTPan {
    Node node;
    int value;
} ASTPan;

typedef struct _ASTDetune {
    Node node;
    int value;
} ASTDetune;

typedef struct _ASTTempo {
    Node node;
    float tempo;
} ASTTempo;

typedef struct _ASTNote {
    Node node;
    char *noteString;
} ASTNote;

typedef struct _ASTRest {
    Node node;
    char *restString;
} ASTRest;

typedef struct _ASTOctave {
    Node node;
    int value;
} ASTOctave;

typedef struct _ASTTranspose {
    Node node;
    bool relative;
    int value;
} ASTTransepose;

typedef struct _ASTTie {
    Node node;
} ASTTie;

typedef struct _ASTGatetime {
    Node node;
    bool absolute;
    int value;
} ASTGatetime;

typedef struct _ASTVelocity {
    Node node;
    bool absolute;
    int value;
} ASTVelocity;

typedef struct _ASTTuplet {
    Node node;
    char *lengthString;
} ASTTuplet;

typedef struct _ASTTrackChange {
    Node node;
} ASTTrackChange;

typedef struct _ASTRepeat {
    Node node;
    int times;
} ASTRepeat;

typedef struct _ASTRepeatBreak {
    Node node;
} ASTRepeatBreak;

typedef struct _ASTChord {
    Node node;
} ASTChord;

typedef struct _ASTVisitor {
    void (*visitRoot)(void *self, ASTRoot *ast);
} ASTVisitor;

extern ASTRoot *MMLASTRootCreate(FileLocation *location);
