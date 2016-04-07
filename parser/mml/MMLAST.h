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

typedef struct _ASTCopyright {
    Node node;
    char *text;
} ASTCopyright;

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
    int bankNo;
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

typedef struct _ASTPitch {
    Node node;
    int value;
    bool coarse;
} ASTPitch;

typedef struct _ASTDetune {
    Node node;
    int value;
} ASTDetune;

typedef struct _ASTPitchSense {
    Node node;
    int value;
} ASTPitchSense;

typedef struct _ASTSustain {
    Node node;
    int value;
} ASTSustain;

typedef struct _ASTTempo {
    Node node;
    float tempo;
} ASTTempo;

typedef struct _ASTTime {
    Node node;
    int numerator;
    int denominator;
} ASTTime;

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
    char direction;
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

typedef struct _ASTLength {
    Node node;
    int length;
} ASTLength;

typedef struct _ASTGatetime {
    Node node;
    bool absolute;
    int value;
} ASTGatetime;

typedef struct _ASTVelocity {
    Node node;
    char direction;
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
    void (*visitTimebase)(void *self, ASTTimebase *ast);
    void (*visitTitle)(void *self, ASTTitle *ast);
    void (*visitCopyright)(void *self, ASTCopyright *ast);
    void (*visitMarker)(void *self, ASTMarker *ast);
    void (*visitVelocityReverse)(void *self, ASTVelocityReverse *ast);
    void (*visitOctaveReverse)(void *self, ASTOctaveReverse *ast);
    void (*visitChannel)(void *self, ASTChannel *ast);
    void (*visitSynth)(void *self, ASTSynth *ast);
    void (*visitBankSelect)(void *self, ASTBankSelect *ast);
    void (*visitProgramChange)(void *self, ASTProgramChange *ast);
    void (*visitVolume)(void *self, ASTVolume *ast);
    void (*visitChorus)(void *self, ASTChorus *ast);
    void (*visitReverb)(void *self, ASTReverb *ast);
    void (*visitExpression)(void *self, ASTExpression *ast);
    void (*visitPan)(void *self, ASTPan *ast);
    void (*visitPitch)(void *self, ASTPitch *ast);
    void (*visitDetune)(void *self, ASTDetune *ast);
    void (*visitPitchSense)(void *self, ASTPitchSense *ast);
    void (*visitSustain)(void *self, ASTSustain *ast);
    void (*visitTempo)(void *self, ASTTempo *ast);
    void (*visitTime)(void *self, ASTTime *ast);
    void (*visitNote)(void *self, ASTNote *ast);
    void (*visitRest)(void *self, ASTRest *ast);
    void (*visitOctave)(void *self, ASTOctave *ast);
    void (*visitTransepose)(void *self, ASTTransepose *ast);
    void (*visitTie)(void *self, ASTTie *ast);
    void (*visitLength)(void *self, ASTLength *ast);
    void (*visitGatetime)(void *self, ASTGatetime *ast);
    void (*visitVelocity)(void *self, ASTVelocity *ast);
    void (*visitTuplet)(void *self, ASTTuplet *ast);
    void (*visitTrackChange)(void *self, ASTTrackChange *ast);
    void (*visitRepeat)(void *self, ASTRepeat *ast);
    void (*visitRepeatBreak)(void *self, ASTRepeatBreak *ast);
    void (*visitChord)(void *self, ASTChord *ast);
} ASTVisitor;

extern ASTRoot *MMLASTRootCreate(FileLocation *location);
extern ASTTimebase *MMLASTTimebaseCreate(FileLocation *location);
extern ASTTitle *MMLASTTitleCreate(FileLocation *location);
extern ASTCopyright *MMLASTCopyrightCreate(FileLocation *location);
extern ASTMarker *MMLASTMarkerCreate(FileLocation *location);
extern ASTVelocityReverse *MMLASTVelocityReverseCreate(FileLocation *location);
extern ASTOctaveReverse *MMLASTOctaveReverseCreate(FileLocation *location);
extern ASTChannel *MMLASTChannelCreate(FileLocation *location);
extern ASTSynth *MMLASTSynthCreate(FileLocation *location);
extern ASTBankSelect *MMLASTBankSelectCreate(FileLocation *location);
extern ASTProgramChange *MMLASTProgramChangeCreate(FileLocation *location);
extern ASTVolume *MMLASTVolumeCreate(FileLocation *location);
extern ASTChorus *MMLASTChorusCreate(FileLocation *location);
extern ASTReverb *MMLASTReverbCreate(FileLocation *location);
extern ASTExpression *MMLASTExpressionCreate(FileLocation *location);
extern ASTPan *MMLASTPanCreate(FileLocation *location);
extern ASTPitch *MMLASTPitchCreate(FileLocation *location);
extern ASTDetune *MMLASTDetuneCreate(FileLocation *location);
extern ASTPitchSense *MMLASTPitchSenseCreate(FileLocation *location);
extern ASTSustain *MMLASTSustainCreate(FileLocation *location);
extern ASTTempo *MMLASTTempoCreate(FileLocation *location);
extern ASTTime *MMLASTTimeCreate(FileLocation *location);
extern ASTNote *MMLASTNoteCreate(FileLocation *location);
extern ASTRest *MMLASTRestCreate(FileLocation *location);
extern ASTOctave *MMLASTOctaveCreate(FileLocation *location);
extern ASTTransepose *MMLASTTranseposeCreate(FileLocation *location);
extern ASTTie *MMLASTTieCreate(FileLocation *location);
extern ASTLength *MMLASTLengthCreate(FileLocation *location);
extern ASTGatetime *MMLASTGatetimeCreate(FileLocation *location);
extern ASTVelocity *MMLASTVelocityCreate(FileLocation *location);
extern ASTTuplet *MMLASTTupletCreate(FileLocation *location);
extern ASTTrackChange *MMLASTTrackChangeCreate(FileLocation *location);
extern ASTRepeat *MMLASTRepeatCreate(FileLocation *location);
extern ASTRepeatBreak *MMLASTRepeatBreakCreate(FileLocation *location);
extern ASTChord *MMLASTChordCreate(FileLocation *location);
