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

typedef struct _ASTMarker {
    Node node;
    char *text;
} ASTMarker;

typedef struct _ASTChannel {
    Node node;
    int number;
} ASTChannel;

typedef struct _ASTVoice {
    Node node;
    int msb;
    int lsb;
    int programNo;
} ASTVoice;

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

typedef struct _ASTInclude {
    Node node;
    char *filepath;
    char *fullpath;
    Node *root;
} ASTInclude;

typedef struct _ASTPattern {
    Node node;
    char *identifier;
} ASTPattern;

typedef struct _ASTDefine {
    Node node;
    char *identifier;
} ASTDefine;

typedef struct _ASTContext {
    Node node;
    NAArray *ctxIdList;
} ASTContext;

typedef struct _ASTIdentifier {
    Node node;
    char *idString;
} ASTIdentifier;

typedef struct _ASTNoteParam {
    Node node;
    int value;
} ASTNoteParam;

typedef struct _ASTVisitor {
    void (*visitRoot)(void *self, ASTRoot *ast);
    void (*visitResolution)(void *self, ASTResolution *ast);
    void (*visitTitle)(void *self, ASTTitle *ast);
    void (*visitTempo)(void *self, ASTTempo *ast);
    void (*visitTime)(void *self, ASTTime *ast);
    void (*visitKey)(void *self, ASTKey *ast);
    void (*visitMarker)(void *self, ASTMarker *ast);
    void (*visitChannel)(void *self, ASTChannel *ast);
    void (*visitVoice)(void *self, ASTVoice *ast);
    void (*visitSynth)(void *self, ASTSynth *ast);
    void (*visitVolume)(void *self, ASTVolume *ast);
    void (*visitPan)(void *self, ASTPan *ast);
    void (*visitChorus)(void *self, ASTChorus *ast);
    void (*visitReverb)(void *self, ASTReverb *ast);
    void (*visitTranspose)(void *self, ASTTranspose *ast);
    void (*visitStep)(void *self, ASTStep *ast);
    void (*visitNote)(void *self, ASTNote *ast);
    void (*visitInclude)(void *self, ASTInclude *ast);
    void (*visitPattern)(void *self, ASTPattern *ast);
    void (*visitDefine)(void *self, ASTDefine *ast);
    void (*visitContext)(void *self, ASTContext *ast);
    void (*visitIdentifier)(void *self, ASTIdentifier *ast);
    void (*visitNoteParam)(void *self, ASTNoteParam *ast);
} ASTVisitor;

extern ASTRoot *NAMidiASTRootCreate(FileLocation *location);
extern ASTResolution *NAMidiASTResolutionCreate(FileLocation *location);
extern ASTTitle *NAMidiASTTitleCreate(FileLocation *location);
extern ASTTempo *NAMidiASTTempoCreate(FileLocation *location);
extern ASTTime *NAMidiASTTimeCreate(FileLocation *location);
extern ASTKey *NAMidiASTKeyCreate(FileLocation *location);
extern ASTMarker *NAMidiASTMarkerCreate(FileLocation *location);
extern ASTChannel *NAMidiASTChannelCreate(FileLocation *location);
extern ASTVoice *NAMidiASTVoiceCreate(FileLocation *location);
extern ASTSynth *NAMidiASTSynthCreate(FileLocation *location);
extern ASTVolume *NAMidiASTVolumeCreate(FileLocation *location);
extern ASTPan *NAMidiASTPanCreate(FileLocation *location);
extern ASTChorus *NAMidiASTChorusCreate(FileLocation *location);
extern ASTReverb *NAMidiASTReverbCreate(FileLocation *location);
extern ASTTranspose *NAMidiASTTransposeCreate(FileLocation *location);
extern ASTStep *NAMidiASTStepCreate(FileLocation *location);
extern ASTNote *NAMidiASTNoteCreate(FileLocation *location);
extern ASTInclude *NAMidiASTIncludeCreate(FileLocation *location);
extern ASTPattern *NAMidiASTPatternCreate(FileLocation *location);
extern ASTDefine *NAMidiASTDefineCreate(FileLocation *location);
extern ASTContext *NAMidiASTContextCreate(FileLocation *location);
extern ASTIdentifier *NAMidiASTIdentifierCreate(FileLocation *location);
extern ASTNoteParam *NAMidiASTNoteParamCreate(FileLocation *location);
