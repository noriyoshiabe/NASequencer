#pragma once

#include "Node.h"

typedef struct _ASTRoot {
    Node node;
} ASTRoot;

typedef struct _ASTResolution {
    Node node;
    int resolution;
} ASTResolution;

typedef struct _ASTTitle {
    Node node;
    char *title;
} ASTTitle;

typedef struct _ASTCopyright {
    Node node;
    char *text;
} ASTCopyright;

typedef struct _ASTTempo {
    Node node;
    float tempo;
} ASTTempo;

typedef struct _ASTTime {
    Node node;
    int numerator;
    int denominator;
} ASTTime;

typedef struct _ASTKey {
    Node node;
    char *keyString;
} ASTKey;

typedef struct _ASTPercussion {
    Node node;
    bool on;
} ASTPercussion;

typedef struct _ASTMarker {
    Node node;
    char *text;
} ASTMarker;

typedef struct _ASTChannel {
    Node node;
    int number;
} ASTChannel;

typedef struct _ASTVelocity {
    Node node;
    int value;
} ASTVelocity;

typedef struct _ASTGatetime {
    Node node;
    bool absolute;
    int value;
} ASTGatetime;

typedef struct _ASTBank {
    Node node;
    int bankNo;
} ASTBank;

typedef struct _ASTProgram {
    Node node;
    int programNo;
} ASTProgram;

typedef struct _ASTSynth {
    Node node;
    char *name;
} ASTSynth;

typedef struct _ASTVolume {
    Node node;
    int value;
} ASTVolume;

typedef struct _ASTPan {
    Node node;
    int value;
} ASTPan;

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

typedef struct _ASTPitch {
    Node node;
    int value;
} ASTPitch;

typedef struct _ASTDetune {
    Node node;
    int value;
} ASTDetune;

typedef struct _ASTPitchSense {
    Node node;
    int value;
} ASTPitchSense;

typedef struct _ASTTranspose {
    Node node;
    int value;
} ASTTranspose;

typedef struct _ASTStep {
    Node node;
    int step;
} ASTStep;

typedef struct _ASTNote {
    Node node;
    char *noteString;
} ASTNote;

typedef struct _ASTPattern {
    Node node;
    char *identifier;
} ASTPattern;

typedef struct _ASTExpand {
    Node node;
    char *identifier;
} ASTExpand;

typedef enum {
    NoteParamTypeVelocity,
    NoteParamTypeGatetime,
} ASTNoteParamType;

typedef struct _ASTNoteParam {
    Node node;
    ASTNoteParamType type;
    int value;
} ASTNoteParam;

typedef struct _ASTVisitor {
    void (*visitRoot)(void *self, ASTRoot *ast);
    void (*visitResolution)(void *self, ASTResolution *ast);
    void (*visitTitle)(void *self, ASTTitle *ast);
    void (*visitCopyright)(void *self, ASTCopyright *ast);
    void (*visitTempo)(void *self, ASTTempo *ast);
    void (*visitTime)(void *self, ASTTime *ast);
    void (*visitKey)(void *self, ASTKey *ast);
    void (*visitPercussion)(void *self, ASTPercussion *ast);
    void (*visitMarker)(void *self, ASTMarker *ast);
    void (*visitChannel)(void *self, ASTChannel *ast);
    void (*visitVelocity)(void *self, ASTVelocity *ast);
    void (*visitGatetime)(void *self, ASTGatetime *ast);
    void (*visitBank)(void *self, ASTBank *ast);
    void (*visitProgram)(void *self, ASTProgram *ast);
    void (*visitSynth)(void *self, ASTSynth *ast);
    void (*visitVolume)(void *self, ASTVolume *ast);
    void (*visitPan)(void *self, ASTPan *ast);
    void (*visitChorus)(void *self, ASTChorus *ast);
    void (*visitReverb)(void *self, ASTReverb *ast);
    void (*visitExpression)(void *self, ASTExpression *ast);
    void (*visitPitch)(void *self, ASTPitch *ast);
    void (*visitDetune)(void *self, ASTDetune *ast);
    void (*visitPitchSense)(void *self, ASTPitchSense *ast);
    void (*visitTranspose)(void *self, ASTTranspose *ast);
    void (*visitStep)(void *self, ASTStep *ast);
    void (*visitNote)(void *self, ASTNote *ast);
    void (*visitPattern)(void *self, ASTPattern *ast);
    void (*visitExpand)(void *self, ASTExpand *ast);
    void (*visitNoteParam)(void *self, ASTNoteParam *ast);
} ASTVisitor;

extern ASTRoot *NAMidiASTRootCreate(FileLocation *location);
extern ASTResolution *NAMidiASTResolutionCreate(FileLocation *location);
extern ASTTitle *NAMidiASTTitleCreate(FileLocation *location);
extern ASTCopyright *NAMidiASTCopyrightCreate(FileLocation *location);
extern ASTTempo *NAMidiASTTempoCreate(FileLocation *location);
extern ASTTime *NAMidiASTTimeCreate(FileLocation *location);
extern ASTKey *NAMidiASTKeyCreate(FileLocation *location);
extern ASTPercussion *NAMidiASTPercussionCreate(FileLocation *location);
extern ASTMarker *NAMidiASTMarkerCreate(FileLocation *location);
extern ASTChannel *NAMidiASTChannelCreate(FileLocation *location);
extern ASTVelocity *NAMidiASTVelocityCreate(FileLocation *location);
extern ASTGatetime *NAMidiASTGatetimeCreate(FileLocation *location);
extern ASTBank *NAMidiASTBankCreate(FileLocation *location);
extern ASTProgram *NAMidiASTProgramCreate(FileLocation *location);
extern ASTSynth *NAMidiASTSynthCreate(FileLocation *location);
extern ASTVolume *NAMidiASTVolumeCreate(FileLocation *location);
extern ASTPan *NAMidiASTPanCreate(FileLocation *location);
extern ASTChorus *NAMidiASTChorusCreate(FileLocation *location);
extern ASTReverb *NAMidiASTReverbCreate(FileLocation *location);
extern ASTExpression *NAMidiASTExpressionCreate(FileLocation *location);
extern ASTPitch *NAMidiASTPitchCreate(FileLocation *location);
extern ASTDetune *NAMidiASTDetuneCreate(FileLocation *location);
extern ASTPitchSense *NAMidiASTPitchSenseCreate(FileLocation *location);
extern ASTTranspose *NAMidiASTTransposeCreate(FileLocation *location);
extern ASTStep *NAMidiASTStepCreate(FileLocation *location);
extern ASTNote *NAMidiASTNoteCreate(FileLocation *location);
extern ASTPattern *NAMidiASTPatternCreate(FileLocation *location);
extern ASTExpand *NAMidiASTExpandCreate(FileLocation *location);
extern ASTNoteParam *NAMidiASTNoteParamCreate(FileLocation *location);

static inline const char *ASTNoteParamType2String(ASTNoteParamType type)
{
#define CASE(type) case type: return &#type[13];
    switch (type) {
    CASE(NoteParamTypeVelocity);
    CASE(NoteParamTypeGatetime);
    }
#undef CASE
}
