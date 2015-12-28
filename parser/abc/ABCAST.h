#pragma once

#include "Node.h"

typedef struct _ASTRoot {
    Node node;
} ASTRoot;

typedef struct _ASTFileIdentification {
    Node node;
    char *identifier;
    int major;
    int minor;
} ASTFileIdentification;

typedef struct _ASTStringInformation {
    Node node;
    char *string;
    char field;
} ASTStringInformation;

typedef struct _ASTReferenceNumber {
    Node node;
    int number;
} ASTReferenceNumber;

typedef struct _ASTTitle {
    Node node;
    char *title;
} ASTTitle;

typedef struct _ASTKey {
    Node node;
} ASTKey;

typedef enum {
    KeyTonic,
    KeyMode,
    KeyAccidental,
    KeyClef,
    KeyMiddle,
    KeyTranspose,
    KeyOctave,
    KeyStaffLines,
} ASTKeyParamType;

typedef struct _ASTKeyParam {
    Node node;
    ASTKeyParamType type;
    char *string;
    int intValue;
} ASTKeyParam;

typedef struct _ASTMeter {
    Node node;
    int numerator;
    int denominator;
    bool free;
    bool commonTime;
    bool cutTime;
} ASTMeter;

typedef struct _ASTUnitNoteLength {
    Node node;
    int numerator;
    int denominator;
} ASTUnitNoteLength;

typedef struct _ASTTempo {
    Node node;
} ASTTempo;

typedef enum {
    TextString,
    BeatUnit,
    BeatCount,
} ASTTempoParamType;

typedef struct _ASTTempoParam {
    Node node;
    ASTTempoParamType type;
    char *string;
    int numerator;
    int denominator;
    int beatCount;
} ASTTempoParam;

typedef struct _ASTParts {
    Node node;
    char *list;
} ASTParts;

typedef struct _ASTInstCharSet {
    Node node;
    char *name;
} ASTInstCharSet;

typedef struct _ASTInstVersion {
    Node node;
    char *numberString;
} ASTInstVersion;

typedef struct _ASTInstInclude {
    Node node;
    char *filepath;
    char *fullpath;
    Node *root;
} ASTInstInclude;

typedef struct _ASTInstCreator {
    Node node;
    char *name;
} ASTInstCreator;

typedef struct _ASTSymbolLine {
    Node node;
    char *string;
} ASTSymbolLine;

typedef struct _ASTContinuation {
    Node node;
    char *string;
} ASTContinuation;

typedef struct _ASTVoice {
    Node node;
    char *identifier;
} ASTVoice;

typedef enum {
    VoiceName,
    VoiceSubname,
    VoiceStemUp,
    VoiceStemDown,
    VoiceClef,
    VoiceMiddle,
    VoiceTranspose,
    VoiceOctave,
    VoiceStaffLines,
} ASTVoiceParamType;

typedef struct _ASTVoiceParam {
    Node node;
    ASTVoiceParamType type;
    char *string;
    int intValue;
} ASTVoiceParam;

typedef struct _ASTTuneBody {
    Node node;
} ASTTuneBody;

typedef struct _ASTLineBreak {
    Node node;
} ASTLineBreak;

typedef struct _ASTAnnotation {
    Node node;
    char *text;
} ASTAnnotation;


typedef struct _ASTVisitor {
    void (*visitRoot)(void *self, ASTRoot *ast);
    void (*visitFileIdentification)(void *self, ASTFileIdentification *ast);
    void (*visitStringInformation)(void *self, ASTStringInformation *ast);
    void (*visitReferenceNumber)(void *self, ASTReferenceNumber *ast);
    void (*visitTitle)(void *self, ASTTitle *ast);
    void (*visitKey)(void *self, ASTKey *ast);
    void (*visitKeyParam)(void *self, ASTKeyParam *ast);
    void (*visitMeter)(void *self, ASTMeter *ast);
    void (*visitUnitNoteLength)(void *self, ASTUnitNoteLength *ast);
    void (*visitTempo)(void *self, ASTTempo *ast);
    void (*visitTempoParam)(void *self, ASTTempoParam *ast);
    void (*visitParts)(void *self, ASTParts *ast);
    void (*visitInstCharSet)(void *self, ASTInstCharSet *ast);
    void (*visitInstVersion)(void *self, ASTInstVersion *ast);
    void (*visitInstInclude)(void *self, ASTInstInclude *ast);
    void (*visitInstCreator)(void *self, ASTInstCreator *ast);
    void (*visitSymbolLine)(void *self, ASTSymbolLine *ast);
    void (*visitContinuation)(void *self, ASTContinuation *ast);
    void (*visitVoice)(void *self, ASTVoice *ast);
    void (*visitVoiceParam)(void *self, ASTVoiceParam *ast);
    void (*visitTuneBody)(void *self, ASTTuneBody *ast);
    void (*visitLineBreak)(void *self, ASTLineBreak *ast);
    void (*visitAnnotation)(void *self, ASTAnnotation *ast);
} ASTVisitor;

extern ASTRoot *ABCASTRootCreate(FileLocation *location);
extern ASTFileIdentification *ABCASTFileIdentificationCreate(FileLocation *location);
extern ASTStringInformation *ABCASTStringInformationCreate(FileLocation *location);
extern ASTReferenceNumber *ABCASTReferenceNumberCreate(FileLocation *location);
extern ASTTitle *ABCASTTitleCreate(FileLocation *location);
extern ASTKey *ABCASTKeyCreate(FileLocation *location);
extern ASTKeyParam *ABCASTKeyParamCreate(FileLocation *location);
extern ASTMeter *ABCASTMeterCreate(FileLocation *location);
extern ASTUnitNoteLength *ABCASTUnitNoteLengthCreate(FileLocation *location);
extern ASTTempo *ABCASTTempoCreate(FileLocation *location);
extern ASTTempoParam *ABCASTTempoParamCreate(FileLocation *location);
extern ASTParts *ABCASTPartsCreate(FileLocation *location);
extern ASTInstCharSet *ABCASTInstCharSetCreate(FileLocation *location);
extern ASTInstVersion *ABCASTInstVersionCreate(FileLocation *location);
extern ASTInstInclude *ABCASTInstIncludeCreate(FileLocation *location);
extern ASTInstCreator *ABCASTInstCreatorCreate(FileLocation *location);
extern ASTSymbolLine *ABCASTSymbolLineCreate(FileLocation *location);
extern ASTContinuation *ABCASTContinuationCreate(FileLocation *location);
extern ASTVoice *ABCASTVoiceCreate(FileLocation *location);
extern ASTVoiceParam *ABCASTVoiceParamCreate(FileLocation *location);
extern ASTTuneBody *ABCASTTuneBodyCreate(FileLocation *location);
extern ASTLineBreak *ABCASTLineBreakCreate(FileLocation *location);
extern ASTAnnotation *ABCASTAnnotationCreate(FileLocation *location);

static inline const char *ASTKeyParamType2String(ASTKeyParamType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(KeyTonic);
    CASE(KeyMode);
    CASE(KeyAccidental);
    CASE(KeyClef);
    CASE(KeyMiddle);
    CASE(KeyTranspose);
    CASE(KeyOctave);
    CASE(KeyStaffLines);
    }
#define undef
}

static inline const char *ASTTempoParamType2String(ASTTempoParamType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(TextString);
    CASE(BeatUnit);
    CASE(BeatCount);
    }
#define undef
}

static inline const char *ASTVoiceParamType2String(ASTVoiceParamType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(VoiceName);
    CASE(VoiceSubname);
    CASE(VoiceStemUp);
    CASE(VoiceStemDown);
    CASE(VoiceClef);
    CASE(VoiceMiddle);
    CASE(VoiceTranspose);
    CASE(VoiceOctave);
    CASE(VoiceStaffLines);
    }
#define undef
}
