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

typedef struct _ASTRest {
    Node node;
    int step;
} ASTRest;

typedef struct _ASTNote {
    Node node;
    char *noteString;
} ASTNote;

typedef struct _ASTInclude {
    Node node;
    char *filepath;
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
    void (*visitRest)(void *self, ASTRest *ast);
    void (*visitNote)(void *self, ASTNote *ast);
    void (*visitInclude)(void *self, ASTInclude *ast);
    void (*visitPattern)(void *self, ASTPattern *ast);
    void (*visitDefine)(void *self, ASTDefine *ast);
    void (*visitContext)(void *self, ASTContext *ast);
    void (*visitIdentifier)(void *self, ASTIdentifier *ast);
    void (*visitNoteParam)(void *self, ASTNoteParam *ast);
} ASTVisitor;

extern ASTRoot *ASTRootCreate(FileLocation *location);
extern ASTResolution *ASTResolutionCreate(FileLocation *location);
extern ASTTitle *ASTTitleCreate(FileLocation *location);
extern ASTTempo *ASTTempoCreate(FileLocation *location);
extern ASTTime *ASTTimeCreate(FileLocation *location);
extern ASTKey *ASTKeyCreate(FileLocation *location);
extern ASTMarker *ASTMarkerCreate(FileLocation *location);
extern ASTChannel *ASTChannelCreate(FileLocation *location);
extern ASTVoice *ASTVoiceCreate(FileLocation *location);
extern ASTSynth *ASTSynthCreate(FileLocation *location);
extern ASTVolume *ASTVolumeCreate(FileLocation *location);
extern ASTPan *ASTPanCreate(FileLocation *location);
extern ASTChorus *ASTChorusCreate(FileLocation *location);
extern ASTReverb *ASTReverbCreate(FileLocation *location);
extern ASTTranspose *ASTTransposeCreate(FileLocation *location);
extern ASTRest *ASTRestCreate(FileLocation *location);
extern ASTNote *ASTNoteCreate(FileLocation *location);
extern ASTInclude *ASTIncludeCreate(FileLocation *location);
extern ASTPattern *ASTPatternCreate(FileLocation *location);
extern ASTDefine *ASTDefineCreate(FileLocation *location);
extern ASTContext *ASTContextCreate(FileLocation *location);
extern ASTIdentifier *ASTIdentifierCreate(FileLocation *location);
extern ASTNoteParam *ASTNoteParamCreate(FileLocation *location);
