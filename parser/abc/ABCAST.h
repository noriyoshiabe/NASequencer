#pragma once

#include "Node.h"

typedef struct _ASTRoot {
    Node node;
} ASTRoot;

typedef struct _ASTVersion {
    Node node;
    char *versionString;
} ASTVersion;

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
    Clef,
    Middle,
    Transpose,
    Octave,
    StaffLines,
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

typedef struct _ASTDecoration {
    Node node;
    char *symbol;
} ASTDecoration;

typedef struct _ASTSymbolLine {
    Node node;
    char *string;
} ASTSymbolLine;

typedef struct _ASTNote {
    Node node;
    char *noteString;
} ASTNote;

typedef struct _ASTLineBreak {
    Node node;
} ASTLineBreak;

typedef struct _ASTVisitor {
    void (*visitRoot)(void *self, ASTRoot *ast);
    void (*visitVersion)(void *self, ASTVersion *ast);
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
    void (*visitDecoration)(void *self, ASTDecoration *ast);
    void (*visitSymbolLine)(void *self, ASTSymbolLine *ast);
    void (*visitNote)(void *self, ASTNote *ast);
    void (*visitLineBreak)(void *self, ASTLineBreak *ast);
} ASTVisitor;

extern ASTRoot *ABCASTRootCreate(FileLocation *location);
extern ASTVersion *ABCASTVersionCreate(FileLocation *location);
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
extern ASTDecoration *ABCASTDecorationCreate(FileLocation *location);
extern ASTSymbolLine *ABCASTSymbolLineCreate(FileLocation *location);
extern ASTNote *ABCASTNoteCreate(FileLocation *location);
extern ASTLineBreak *ABCASTLineBreakCreate(FileLocation *location);

static inline const char *ASTKeyParamType2String(ASTKeyParamType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(KeyTonic);
    CASE(KeyMode);
    CASE(KeyAccidental);
    CASE(Clef);
    CASE(Middle);
    CASE(Transpose);
    CASE(Octave);
    CASE(StaffLines);
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
