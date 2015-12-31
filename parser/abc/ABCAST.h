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
    float beatCount;
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

typedef struct _ASTInstLineBreak {
    Node node;
    char character;
} ASTInstLineBreak;

typedef struct _ASTInstDecoration {
    Node node;
    char character;
} ASTInstDecoration;

typedef struct _ASTMacro {
    Node node;
    char *target;
    char *replacement;
} ASTMacro;

typedef struct _ASTSymbolLine {
    Node node;
    char *string;
} ASTSymbolLine;

typedef struct _ASTRedefinableSymbol {
    Node node;
    char *symbol;
    char *replacement;
} ASTRedefinableSymbol;

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

typedef struct _ASTDecoration {
    Node node;
    char *symbol;
} ASTDecoration;

typedef struct _ASTNote {
    Node node;
    char *noteString;
} ASTNote;

typedef struct _ASTBrokenRhythm {
    Node node;
    char direction;
} ASTBrokenRhythm;

typedef struct _ASTRest {
    Node node;
    char *restString;
} ASTRest;

typedef struct _ASTRepeatBar {
    Node node;
    char *symbols;
} ASTRepeatBar;

typedef struct _ASTTie {
    Node node;
} ASTTie;

typedef struct _ASTSlur {
    Node node;
    char direction;
} ASTSlur;

typedef struct _ASTDot {
    Node node;
} ASTDot;

typedef struct _ASTGraceNote {
    Node node;
    bool acciaccatura;
} ASTGraceNote;

typedef struct _ASTTuplet {
    Node node;
    char *tupletString;
} ASTTuplet;

typedef struct _ASTChord {
    Node node;
    char *lengthString;
} ASTChord;

typedef struct _ASTOverlay {
    Node node;
} ASTOverlay;

typedef struct _ASTMidi {
    Node node;
} ASTMidi;

typedef enum {
    MidiVoiceId,
    MidiInstrument,
    MidiBank,
    MidiMute,
} ASTMidiParamType;

typedef struct _ASTMidiParam {
    Node node;
    ASTMidiParamType type;
    char *string;
    int intValue;
} ASTMidiParam;

typedef enum {
    PropagateAccidentalNot,
    PropagateAccidentalOctave,
    PropagateAccidentalPitch,
} ASTPropagateAccidentalType;

typedef struct _ASTPropagateAccidental {
    Node node;
    ASTPropagateAccidentalType type;
} ASTPropagateAccidental;


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
    void (*visitInstLineBreak)(void *self, ASTInstLineBreak *ast);
    void (*visitInstDecoration)(void *self, ASTInstDecoration *ast);
    void (*visitMacro)(void *self, ASTMacro *ast);
    void (*visitSymbolLine)(void *self, ASTSymbolLine *ast);
    void (*visitRedefinableSymbol)(void *self, ASTRedefinableSymbol *ast);
    void (*visitContinuation)(void *self, ASTContinuation *ast);
    void (*visitVoice)(void *self, ASTVoice *ast);
    void (*visitVoiceParam)(void *self, ASTVoiceParam *ast);
    void (*visitTuneBody)(void *self, ASTTuneBody *ast);
    void (*visitLineBreak)(void *self, ASTLineBreak *ast);
    void (*visitAnnotation)(void *self, ASTAnnotation *ast);
    void (*visitDecoration)(void *self, ASTDecoration *ast);
    void (*visitNote)(void *self, ASTNote *ast);
    void (*visitBrokenRhythm)(void *self, ASTBrokenRhythm *ast);
    void (*visitRest)(void *self, ASTRest *ast);
    void (*visitRepeatBar)(void *self, ASTRepeatBar *ast);
    void (*visitTie)(void *self, ASTTie *ast);
    void (*visitSlur)(void *self, ASTSlur *ast);
    void (*visitDot)(void *self, ASTDot *ast);
    void (*visitGraceNote)(void *self, ASTGraceNote *ast);
    void (*visitTuplet)(void *self, ASTTuplet *ast);
    void (*visitChord)(void *self, ASTChord *ast);
    void (*visitOverlay)(void *self, ASTOverlay *ast);
    void (*visitMidi)(void *self, ASTMidi *ast);
    void (*visitMidiParam)(void *self, ASTMidiParam *ast);
    void (*visitPropagateAccidental)(void *self, ASTPropagateAccidental *ast);
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
extern ASTInstLineBreak *ABCASTInstLineBreakCreate(FileLocation *location);
extern ASTInstDecoration *ABCASTInstDecorationCreate(FileLocation *location);
extern ASTMacro *ABCASTMacroCreate(FileLocation *location);
extern ASTSymbolLine *ABCASTSymbolLineCreate(FileLocation *location);
extern ASTRedefinableSymbol *ABCASTRedefinableSymbolCreate(FileLocation *location);
extern ASTContinuation *ABCASTContinuationCreate(FileLocation *location);
extern ASTVoice *ABCASTVoiceCreate(FileLocation *location);
extern ASTVoiceParam *ABCASTVoiceParamCreate(FileLocation *location);
extern ASTTuneBody *ABCASTTuneBodyCreate(FileLocation *location);
extern ASTLineBreak *ABCASTLineBreakCreate(FileLocation *location);
extern ASTAnnotation *ABCASTAnnotationCreate(FileLocation *location);
extern ASTDecoration *ABCASTDecorationCreate(FileLocation *location);
extern ASTNote *ABCASTNoteCreate(FileLocation *location);
extern ASTBrokenRhythm *ABCASTBrokenRhythmCreate(FileLocation *location);
extern ASTRest *ABCASTRestCreate(FileLocation *location);
extern ASTRepeatBar *ABCASTRepeatBarCreate(FileLocation *location);
extern ASTTie *ABCASTTieCreate(FileLocation *location);
extern ASTSlur *ABCASTSlurCreate(FileLocation *location);
extern ASTDot *ABCASTDotCreate(FileLocation *location);
extern ASTGraceNote *ABCASTGraceNoteCreate(FileLocation *location);
extern ASTTuplet *ABCASTTupletCreate(FileLocation *location);
extern ASTChord *ABCASTChordCreate(FileLocation *location);
extern ASTOverlay *ABCASTOverlayCreate(FileLocation *location);
extern ASTMidi *ABCASTMidiCreate(FileLocation *location);
extern ASTMidiParam *ABCASTMidiParamCreate(FileLocation *location);
extern ASTPropagateAccidental *ABCASTPropagateAccidentalCreate(FileLocation *location);


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
#undef CASE
}

static inline const char *ASTTempoParamType2String(ASTTempoParamType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(TextString);
    CASE(BeatUnit);
    CASE(BeatCount);
    }
#undef CASE
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
#undef CASE
}

static inline const char *ASTMidiParamType2String(ASTMidiParamType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(MidiVoiceId);
    CASE(MidiInstrument);
    CASE(MidiBank);
    CASE(MidiMute);
    }
#undef CASE
}

static inline const char *ASTPropagateAccidentalType2String(ASTPropagateAccidentalType type)
{
#define CASE(type) case type: return #type;
    switch (type) {
    CASE(PropagateAccidentalNot);
    CASE(PropagateAccidentalOctave);
    CASE(PropagateAccidentalPitch);
    }
#undef CASE
}
