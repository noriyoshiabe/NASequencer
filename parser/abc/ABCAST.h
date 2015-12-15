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

typedef struct _ASTMeter {
    Node node;
    int numerator;
    int denominator;
    bool free;
    bool commonTime;
    bool cutTime;
} ASTMeter;

typedef struct _ASTKeyParam {
    Node node;
    ASTKeyParamType type;
    char *string;
    int intValue;
} ASTKeyParam;

typedef struct _ASTNote {
    Node node;
    char *noteString;
} ASTNote;

typedef struct _ASTLineBreak {
    Node node;
} ASTLineBreak;

typedef struct _ASTInclude {
    Node node;
    char *filepath;
    char *fullpath;
    Node *root;
} ASTInclude;

typedef struct _ASTVisitor {
    void (*visitRoot)(void *self, ASTRoot *ast);
    void (*visitVersion)(void *self, ASTVersion *ast);
    void (*visitStringInformation)(void *self, ASTStringInformation *ast);
    void (*visitReferenceNumber)(void *self, ASTReferenceNumber *ast);
    void (*visitTitle)(void *self, ASTTitle *ast);
    void (*visitKey)(void *self, ASTKey *ast);
    void (*visitKeyParam)(void *self, ASTKeyParam *ast);
    void (*visitMeter)(void *self, ASTMeter *ast);
    void (*visitNote)(void *self, ASTNote *ast);
    void (*visitLineBreak)(void *self, ASTLineBreak *ast);
    void (*visitInclude)(void *self, ASTInclude *ast);
} ASTVisitor;

extern ASTRoot *ABCASTRootCreate(FileLocation *location);
extern ASTVersion *ABCASTVersionCreate(FileLocation *location);
extern ASTStringInformation *ABCASTStringInformationCreate(FileLocation *location);
extern ASTReferenceNumber *ABCASTReferenceNumberCreate(FileLocation *location);
extern ASTTitle *ABCASTTitleCreate(FileLocation *location);
extern ASTKey *ABCASTKeyCreate(FileLocation *location);
extern ASTKeyParam *ABCASTKeyParamCreate(FileLocation *location);
extern ASTMeter *ABCASTMeterCreate(FileLocation *location);
extern ASTNote *ABCASTNoteCreate(FileLocation *location);
extern ASTLineBreak *ABCASTLineBreakCreate(FileLocation *location);
extern ASTInclude *ABCASTIncludeCreate(FileLocation *location);

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
