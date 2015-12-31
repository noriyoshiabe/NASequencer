#include "ABCASTAnalyzer.h"
#include "ABCParser.h"
#include "ABCAST.h"
#include "ABCSEM.h"
#include "NoteTable.h"
#include "NACString.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#define node(type, ast) ABCSEM##type##Create(&ast->node.location)
#define append(node, sem) NAArrayAppend(((Node *)(node))->children, sem)
#define appendError(self, ast, ...) self->context->appendError(self->context, &ast->node.location, __VA_ARGS__)
#define TuneOrFile(self) (self->tune ? (Node *)self->tune : (Node *)self->file)
#define NoteTarget(self) \
    (self->chord ? (Node *)self->chord : \
     self->graceNote ? (Node *)self->graceNote : \
     self->voice ? (Node *)self->voice : \
     self->part ? (Node *)self->part : \
     (Node *)self->tune)

#define isValidRange(v, from, to) (from <= v && v <= to)
#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))
#define RESOLUTION 480

#define __Trace__ printf("-- %s:%s - %d\n", __FILE__, __func__, __LINE__);

typedef enum {
    FileHeader,
    TuneHeader,
    TuneBody,
} State;

static inline const char *State2String(State state)
{
#define CASE(state) case state: return #state;
    switch (state) {
    CASE(FileHeader);
    CASE(TuneHeader);
    CASE(TuneBody);
    }
#undef CASE
}

typedef struct _ABCASTAnalyzer {
    ASTVisitor visitor;
    Analyzer analyzer;

    ParseContext *context;
    State state;

    SEMFile *file;
    SEMTune *tune;
    SEMKey *key;
    SEMTempo *tempo;
    SEMPart *part;
    SEMVoice *voice;
    SEMGraceNote *graceNote;
    SEMChord *chord;

    SEMMeter *fileMeter;
    SEMMeter *tuneMeter;

    regex_t nthRepeatRegex;
} ABCASTAnalyzer;

static BaseNote KeyChar2BaseNote(char c);
static bool parseNoteLength(char *_string, NoteLength *noteLength);

static Node *process(void *_self, Node *node)
{
    ABCASTAnalyzer *self = _self;

    SEMFile *file = ABCSEMFileCreate(NULL);
    file->node.children = NAArrayCreate(4, NADescriptionAddress);

    self->file = file;
    node->accept(node, self);

    return (Node *)file;
}

static void destroy(void *_self)
{
    ABCASTAnalyzer *self = _self;
    regfree(&self->nthRepeatRegex);
    free(self);
}

static void visitRoot(void *self, ASTRoot *ast)
{
    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitFileIdentification(void *self, ASTFileIdentification *ast)
{
}

static void visitStringInformation(void *self, ASTStringInformation *ast)
{
}

static void visitReferenceNumber(void *_self, ASTReferenceNumber *ast)
{
    ABCASTAnalyzer *self = _self;

    SEMTune *tune = node(Tune, ast);
    tune->number = ast->number;

    append(self->file, tune);
    self->tune = tune;

    self->state = TuneHeader;
    self->tuneMeter = self->fileMeter;
}

static void visitTitle(void *_self, ASTTitle *ast)
{
    ABCASTAnalyzer *self = _self;
    
    if (!self->tune) {
        appendError(self, ast, ABCParseErrorIllegalStateWithTitle, State2String(self->state), NULL);
        return;
    }

    NAArrayAppend(self->tune->titleList, strdup(ast->title));
}

static void visitKey(void *_self, ASTKey *ast)
{
    ABCASTAnalyzer *self = _self;
    NAIterator *iterator;

    if (!self->tune) {
        appendError(self, ast, ABCParseErrorIllegalStateWithKey, State2String(self->state), NULL);
        return;
    }

    self->state = TuneBody;

    SEMKey *key = node(Key, ast);
    self->key = key;

    iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    BaseNote baseNote = BaseNote_C;
    bool sharp = false;
    bool flat = false;
    Mode mode = ModeMajor;

    if (key->tonic) {
        char *_tonic = NACStringDuplicate(key->tonic->string);
        NACStringToLowerCase(_tonic);

        if (0 == strcmp("none", _tonic)) {
            goto KEY_FOUND;
        }

        if (0 == strcmp("hp", _tonic)) {
            baseNote = BaseNote_D;
            goto KEY_FOUND;
        }

        baseNote = KeyChar2BaseNote(_tonic[0]);
        sharp = NULL != strchr(&_tonic[1], '#');
        flat = NULL != strchr(&_tonic[1], 'b');
    }

    if (key->mode) {
        char *_mode = NACStringDuplicate(key->mode->string);
        NACStringToLowerCase(_mode);

        const struct {
            const char *name;
            Mode mode;
        } scales[] = {
            {"ma", ModeMajor}, {"maj", ModeMajor}, {"major", ModeMajor},
            {"m", ModeMinor}, {"min", ModeMinor}, {"minor", ModeMinor},
            {"ion", ModeIonian}, {"ionian", ModeIonian},
            {"aeo", ModeAeolian}, {"aeolian", ModeAeolian},
            {"mix", ModeMixolydian}, {"mixolydian", ModeMixolydian},
            {"dor", ModeDorian}, {"dorian", ModeDorian},
            {"phr", ModePhrygian}, {"phrygian", ModePhrygian},
            {"lyd", ModeLydian}, {"lydian", ModeLydian},
            {"loc", ModeLocrian}, {"locrian", ModeLocrian},
            {"exp", ModeMajor}, {"explicit", ModeMajor},
        };

        for (int i = 0; i < sizeof(scales)/sizeof(scales[0]); ++i) {
            if (0 == strcmp(_mode, scales[i].name)) {
                mode = scales[i].mode;
                goto KEY_FOUND;
            }
        }

        appendError(self, key->mode, ABCParseErrorInvalidKeyMode, key->mode->string, NULL);
        NodeRelease(key);
        return;
    }

KEY_FOUND:
    ;

    NoteTable *noteTable = NoteTableCreate(baseNote, sharp, flat, mode);
    if (NoteTableHasUnusualKeySign(noteTable)) {
        char *_tonic = key->tonic ? key->tonic->string : "none";
        char *_mode = key->mode ? key->mode->string : "none";
        appendError(self, ast, ABCParseErrorInvalidKey, _tonic, _mode, NULL);
        NoteTableRelease(noteTable);
        NodeRelease(key);
        return;
    }

    iterator = NAArrayGetIterator(key->accidentals);
    while (iterator->hasNext(iterator)) {
        ASTKeyParam *astAccidental = iterator->next(iterator);
        char *pc = astAccidental->string;
        char c;

        BaseNote baseNote = BaseNote_C;
        Accidental accidental = AccidentalNone;

        while ((c = *(pc++))) {
            switch (c) {
            case '^':
                accidental = AccidentalSharp == accidental ? AccidentalDoubleSharp : AccidentalSharp;
                break;
            case '_':
                accidental = AccidentalFlat == accidental ? AccidentalDoubleFlat : AccidentalFlat;
                break;
            case '=':
                accidental = AccidentalNatural;
                break;
            default:
                baseNote = KeyChar2BaseNote(tolower(c));
                break;
            }
        }

        NoteTableAppendAccidental(noteTable, baseNote, accidental);
    }

    key->noteTable = noteTable;
    append(self->tune, key);
}

static void visitKeyParam(void *_self, ASTKeyParam *ast)
{
    ABCASTAnalyzer *self = _self;
    
    switch (ast->type) {
    case KeyTonic:
        if (self->key->tonic) {
            appendError(self, ast, ABCParseErrorDuplicatedKeyTonic, ast->string, NULL);
        }
        else {
            self->key->tonic = ast;
        }
        break;
    case KeyMode:
        if (self->key->mode) {
            appendError(self, ast, ABCParseErrorDuplicatedKeyMode, ast->string, NULL);
        }
        else {
            self->key->mode = ast;
        }
        break;
    case KeyAccidental:
        NAArrayAppend(self->key->accidentals, ast);
        break;
    case KeyClef:
    case KeyMiddle:
        break;
    case KeyTranspose:
        if (!isValidRange(ast->intValue, -64, 64)) {
            appendError(self, ast, ABCParseErrorInvalidTranspose, NACStringFromInteger(ast->intValue), NULL);
        }
        else {
            self->key->transpose = ast->intValue;
        }
        break;
    case KeyOctave:
        if (!isValidRange(ast->intValue, -3, 3)) {
            appendError(self, ast, ABCParseErrorInvalidOctave, NACStringFromInteger(ast->intValue), NULL);
        }
        else {
            self->key->octave = ast->intValue;
        }
        break;
    case KeyStaffLines:
        break;
    }
}

static void visitMeter(void *_self, ASTMeter *ast)
{
    ABCASTAnalyzer *self = _self;

    int numerator = ast->numerator;
    int denominator = ast->denominator;

    if (ast->free || ast->commonTime) {
        numerator = 4;
        denominator = 4;
    }
    else if (ast->cutTime) {
        numerator = 2;
        denominator = 2;
    }

    if (1 > numerator || 1 > denominator || !isPowerOf2(denominator)) {
        appendError(self, ast, ABCParseErrorInvalidMeter, NACStringFromInteger(numerator), NACStringFromInteger(denominator), NULL);
    }
    else {
        SEMMeter *meter = node(Meter, ast);
        meter->numerator = numerator;
        meter->denominator = denominator;

        append(TuneOrFile(self), meter);

        if (FileHeader == self->state) {
            self->fileMeter = meter;
        }
        else {
            self->tuneMeter = meter;
        }
    }
}

static void visitUnitNoteLength(void *_self, ASTUnitNoteLength *ast)
{
    ABCASTAnalyzer *self = _self;

    int numerator = ast->numerator;
    int denominator = -1 == ast->denominator ? 1 : ast->denominator;

    if (1 > numerator || 1 > denominator
            || 0 != RESOLUTION * 4 * numerator % denominator) {
        appendError(self, ast, ABCParseErrorInvalidUnitNoteLength, NACStringFromInteger(ast->numerator), NACStringFromInteger(ast->denominator), NULL);
        return;
    }

    SEMUnitNoteLength *unLength = node(UnitNoteLength, ast);
    unLength->length = RESOLUTION * 4 * numerator / denominator;

    append(TuneOrFile(self), unLength);
}

static void visitTempo(void *_self, ASTTempo *ast)
{
    ABCASTAnalyzer *self = _self;

    SEMTempo *tempo = node(Tempo, ast);
    self->tempo = tempo;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    if (!self->tempo->beatCount) {
        NodeRelease(tempo);
        return;
    }

    int unitLength = 0;

    if (NAArrayIsEmpty(self->tempo->beatUnits)) {
        unitLength = RESOLUTION;
    }
    else {
        NAIterator *iterator = NAArrayGetIterator(self->tempo->beatUnits);
        while (iterator->hasNext(iterator)) {
            ASTTempoParam *beatUnit = iterator->next(iterator);
            int numerator = beatUnit->numerator;
            int denominator = beatUnit->denominator;

            if (1 > numerator || 1 > denominator
                    || 0 != RESOLUTION * 4 * numerator % denominator) {
                appendError(self, ast, ABCParseErrorInvalidBeatUnit, NACStringFromInteger(numerator), NACStringFromInteger(denominator), NULL);
                NodeRelease(tempo);
                return;
            }

            unitLength += RESOLUTION * 4 * numerator / denominator;
        }
    }

    float _tempo = self->tempo->beatCount->beatCount * unitLength / RESOLUTION;
    if (!isValidRange(_tempo, 30.0, 300.0)) {
        appendError(self, ast, ABCParseErrorInvalidTempo, NACStringFromFloat(_tempo, 2), NULL);
        NodeRelease(tempo);
        return;
    }

    tempo->tempo = _tempo;
    append(TuneOrFile(self), tempo);
}

static void visitTempoParam(void *_self, ASTTempoParam *ast)
{
    ABCASTAnalyzer *self = _self;

    switch (ast->type) {
    case TextString:
        break;
    case BeatUnit:
        if (!self->tempo->beatCount) {
            NAArrayAppend(self->tempo->beatUnits, ast);
        }
        break;
    case BeatCount:
        if (!self->tempo->beatCount) {
            if (1.f > ast->beatCount) {
                appendError(self, ast, ABCParseErrorInvalidBeatCount, NACStringFromFloat(ast->beatCount, 2), NULL);
            }
            else {
                self->tempo->beatCount = ast;
            }
        }
        break;
    }
}

static void visitParts(void *_self, ASTParts *ast)
{
    ABCASTAnalyzer *self = _self;

    switch (self->state) {
    case FileHeader:
        appendError(self, ast, ABCParseErrorIllegalStateWithParts, State2String(self->state), NULL);
        return;
    case TuneHeader:
        if (self->tune->partSequence) {
            free(self->tune->partSequence);
        }
        self->tune->partSequence = strdup(ast->list);
        break;
    case TuneBody:
        {
            char identifier[2] = {ast->list[0], '\0'};
            SEMPart *part = NAMapGet(self->tune->partMap, identifier);
            if (!part) {
                part = node(Part, ast);
                part->identifier = strdup(identifier);
                NAMapPut(self->tune->partMap, part->identifier, part);
            }
            self->part = part;
            self->voice = NULL;
        }
        break;
    }
}

static void visitInstCharSet(void *self, ASTInstCharSet *ast)
{
}

static void visitInstVersion(void *self, ASTInstVersion *ast)
{
}

static void visitInstInclude(void *_self, ASTInstInclude *ast)
{
    ABCASTAnalyzer *self = _self;

    switch (self->state) {
    case FileHeader:
    case TuneHeader:
        if (ast->root) {
            ast->root->accept(ast->root, self);
        }
        break;
    case TuneBody:
        appendError(self, ast, ABCParseErrorIllegalStateWithInclude, State2String(self->state), NULL);
        break;
    }

}

static void visitInstCreator(void *self, ASTInstCreator *ast)
{
}

static void visitInstLineBreak(void *self, ASTInstLineBreak *ast)
{
}

static void visitInstDecoration(void *self, ASTInstDecoration *ast)
{
}

static void visitMacro(void *self, ASTMacro *ast)
{
}

static void visitSymbolLine(void *self, ASTSymbolLine *ast)
{
}

static void visitRedefinableSymbol(void *self, ASTRedefinableSymbol *ast)
{
}

static void visitContinuation(void *self, ASTContinuation *ast)
{
}

static void visitVoice(void *_self, ASTVoice *ast)
{
    ABCASTAnalyzer *self = _self;

    switch (self->state) {
    case FileHeader:
        appendError(self, ast, ABCParseErrorIllegalStateWithVoice, State2String(self->state), NULL);
        break;
    case TuneHeader:
    case TuneBody:
        {
            NAMap *voiceMap = self->part ? self->part->voiceMap : self->tune->voiceMap;

            SEMVoice *voice = NAMapGet(voiceMap, ast->identifier);
            if (!voice) {
                voice = node(Voice, ast);
                voice->identifier = strdup(ast->identifier);
                NAMapPut(voiceMap, voice->identifier, voice);
            }
            
            self->voice = voice;

            NAIterator *iterator = NAArrayGetIterator(ast->node.children);
            while (iterator->hasNext(iterator)) {
                Node *node = iterator->next(iterator);
                node->accept(node, self);
            }

            if (TuneHeader == self->state) {
                self->voice = NULL;
            }
        }
        break;
    }
}

static void visitVoiceParam(void *_self, ASTVoiceParam *ast)
{
    ABCASTAnalyzer *self = _self;

    switch (ast->type) {
    case VoiceName:
    case VoiceSubname:
    case VoiceStemUp:
    case VoiceStemDown:
    case VoiceClef:
    case VoiceMiddle:
        break;
    case VoiceTranspose:
        if (!isValidRange(ast->intValue, -64, 64)) {
            appendError(self, ast, ABCParseErrorInvalidTranspose, NACStringFromInteger(ast->intValue), NULL);
        }
        else {
            self->voice->transpose = ast->intValue;
        }
        break;
    case VoiceOctave:
        if (!isValidRange(ast->intValue, -3, 3)) {
            appendError(self, ast, ABCParseErrorInvalidOctave, NACStringFromInteger(ast->intValue), NULL);
        }
        else {
            self->voice->octave = ast->intValue;
        }
        break;
    case VoiceStaffLines:
        break;
    }
}

static void visitTuneBody(void *_self, ASTTuneBody *ast)
{
    ABCASTAnalyzer *self = _self;

    if (TuneBody != self->state) {
        appendError(self, ast, ABCParseErrorIllegalStateWithTuneBody, State2String(self->state), NULL);
        return;
    }

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitLineBreak(void *self, ASTLineBreak *ast)
{
}

static void visitAnnotation(void *self, ASTAnnotation *ast)
{
}

static void visitDecoration(void *self, ASTDecoration *ast)
{
    // TODO expand to instructions like pianissimo, forte and so on
}

static void visitNote(void *_self, ASTNote *ast)
{
    ABCASTAnalyzer *self = _self;

    BaseNote baseNote = BaseNote_C;
    Accidental accidental = AccidentalNone;
    int octave = 2;
    NoteLength noteLength = {1, 1};

    char *pc = ast->noteString;
    while (*pc) {
        switch (*pc) {
        case '^':
            accidental = AccidentalSharp == accidental ? AccidentalDoubleSharp : AccidentalSharp;
            break;
        case '_':
            accidental = AccidentalFlat == accidental ? AccidentalDoubleFlat : AccidentalFlat;
            break;
        case '=':
            accidental = AccidentalNatural;
            break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
            baseNote = KeyChar2BaseNote(*pc);
            octave = isupper(*pc) ? 2 : 3;
            break;
        case ',':
            --octave;
            break;
        case '\'':
            ++octave;
            break;
        default:
            if (!parseNoteLength(pc, &noteLength)) {
                appendError(self, ast, ABCParseErrorInvalidNoteLength, pc, NULL);
                return;
            }

            goto LOOP_END;
        }

        ++pc;
    }

LOOP_END:
    ;

    SEMNote *note = node(Note, ast);
    note->baseNote = baseNote;
    note->accidental = accidental;
    note->octave = octave;
    note->length = noteLength;

    append(NoteTarget(self), note);
}

static void visitBrokenRhythm(void *_self, ASTBrokenRhythm *ast)
{
    ABCASTAnalyzer *self = _self;

    SEMBrokenRhythm *bRhythm = node(BrokenRhythm, ast);
    bRhythm->direction = ast->direction;
    append(NoteTarget(self), bRhythm);
}

static void visitRest(void *_self, ASTRest *ast)
{
    ABCASTAnalyzer *self = _self;

    SEMRestType type = isupper(ast->restString[0]) ? RestMeasure : RestUnitNote;
    NoteLength noteLength;

    if (!parseNoteLength(ast->restString + 1, &noteLength)) {
        appendError(self, ast, ABCParseErrorInvalidNoteLength, ast->restString + 1, NULL);
        return;
    }

    SEMRest *rest = node(Rest, ast);
    rest->type = type;
    rest->length = noteLength;
    append(NoteTarget(self), rest);
}

static bool parseNThRepeat(char *_string, NASet **nthSet)
{
    char *string = NACStringDuplicate(_string);
    char *saveptr, *token;

    NASet *set = NASetCreate(NAHashCInteger, NADescriptionCInteger);

    while ((token = strtok_r(string, ",", &saveptr))) {
        char *ranges[2];
        int count = 0;
        char *_saveptr, *_token;
        while ((_token = strtok_r(token, "-", &_saveptr))) {
            if (2 <= count) {
                goto ERROR;
            }

            ranges[count++] = _token;
            token = NULL;
        }

        switch (count) {
        case 1:
            {
                int nth = atoi(ranges[0]);
                if (1 > nth) {
                    goto ERROR;
                }

                int *value = malloc(sizeof(int));
                *value = nth;
                NASetAdd(set, value);
            }
            break;
        case 2:
            {
                int from = atoi(ranges[0]);
                int to = atoi(ranges[1]);
                if (1 > from || from > to) {
                    goto ERROR;
                }

                for (int i = from; i <= to; ++i) {
                    int *value = malloc(sizeof(int));
                    *value = i;
                    NASetAdd(set, value);
                }
            }
            break;
        default:
            break;
        }
    
        string = NULL;
    }

    *nthSet = set;
    return true;

ERROR:
    NASetTraverse(set, free);
    NASetDestroy(set);
    return false;
}

static void visitRepeatBar(void *_self, ASTRepeatBar *ast)
{
    ABCASTAnalyzer *self = _self;

    bool hasRepeatStart = false;
    bool hasRepeatEnd = false;
    bool hasBarLine = false;

    char *pRepeatStart = strstr(ast->symbols, "|:");
    char *pRepeatEnd = strstr(ast->symbols, ":|");
    if (pRepeatStart && pRepeatEnd) {
        if (pRepeatStart < pRepeatEnd) {
            appendError(self, ast, ABCParseErrorInvalidRepeat, ast->symbols, NULL);
            return;
        }

        hasRepeatStart = true;
        hasRepeatEnd = true;
    }
    else if (pRepeatStart) {
        hasRepeatStart = true;
    }
    else if (pRepeatEnd) {
        hasRepeatEnd = true;
    }

    if (!hasRepeatStart && !hasRepeatEnd) {
        if (strstr(ast->symbols, "::")) {
            hasRepeatStart = true;
            hasRepeatEnd = true;
        }
        else if (strchr(ast->symbols, '|')) {
            hasBarLine = true;
        }
    }

    if (hasRepeatEnd) {
        SEMRepeat *repeat = node(Repeat, ast);
        repeat->type = RepeatEnd;
        append(NoteTarget(self), repeat);
    }

    if (hasRepeatStart) {
        SEMRepeat *repeat = node(Repeat, ast);
        repeat->type = RepeatStart;
        append(NoteTarget(self), repeat);
    }

    if (hasBarLine) {
        SEMBarLine *barLine = node(BarLine, ast);
        append(NoteTarget(self), barLine);
    }

    char *pDigit = strpbrk(ast->symbols, "0123456789");
    if (pDigit) {
        if ('0' == *pDigit) {
            appendError(self, ast, ABCParseErrorInvalidNthRepeat, pDigit, NULL);
            return;
        }

        regmatch_t match[1];

        if (REG_NOMATCH == regexec(&self->nthRepeatRegex, ast->symbols, 1, match, 0)) {
            appendError(self, ast, ABCParseErrorInvalidNthRepeat, ast->symbols, NULL);
        }
        else {
            NASet *nthSet;
            if (!parseNThRepeat(pDigit, &nthSet)) {
                appendError(self, ast, ABCParseErrorInvalidNthRepeat, pDigit, NULL);
                return;
            }

            SEMRepeat *repeat = node(Repeat, ast);
            repeat->type = RepeatNth;
            repeat->nthSet = nthSet;
            append(NoteTarget(self), repeat);
        }
    }
}

static void visitTie(void *_self, ASTTie *ast)
{
    ABCASTAnalyzer *self = _self;

    SEMTie *tie = node(Tie, ast);
    append(NoteTarget(self), tie);
}

static void visitSlur(void *self, ASTSlur *ast)
{
}

static void visitDot(void *self, ASTDot *ast)
{
}

static void visitGraceNote(void *_self, ASTGraceNote *ast)
{
    ABCASTAnalyzer *self = _self;

    SEMGraceNote *graceNote = node(GraceNote, ast);
    self->graceNote = graceNote;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->graceNote = NULL;
    append(NoteTarget(self), graceNote);
}

static void visitTuplet(void *_self, ASTTuplet *ast)
{
    ABCASTAnalyzer *self = _self;

    SEMTuplet *tuplet = node(Tuplet, ast);

    char *string = NACStringDuplicate(ast->tupletString + 1);
    int params[3];
    int count = 0;
    char *saveptr, *token;
    while ((token = strtok_r(string, ":", &saveptr))) {
        params[count++] = atoi(token);
        string = NULL;
    }

    tuplet->division = params[0];

    if (1 < count) {
        tuplet->time = params[1];
    }
    else {
        if (self->tuneMeter) {
            int n = self->tuneMeter->numerator;
            int d = self->tuneMeter->denominator;
            if (8 == d && (6 == n || 9 == n || 12 == n)) {
                tuplet->time = 3;
            }
            else {
                tuplet->time = 2;
            }
        }
        else {
            tuplet->time = 2;
        }
    }

    if (2 < count) {
        tuplet->count = params[2];
    }
    else {
        tuplet->count = tuplet->division;
    }

    append(NoteTarget(self), tuplet);
}

static void visitChord(void *_self, ASTChord *ast)
{
    ABCASTAnalyzer *self = _self;

    NoteLength noteLength;
    if (!parseNoteLength(ast->lengthString, &noteLength)) {
        appendError(self, ast, ABCParseErrorInvalidNoteLength, ast->lengthString, NULL);
        return;
    }

    SEMChord *chord = node(Chord, ast);
    chord->length = noteLength;

    self->chord = chord;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->chord = NULL;

    append(NoteTarget(self), chord);
}

static void visitOverlay(void *_self, ASTOverlay *ast)
{
    ABCASTAnalyzer *self = _self;
    SEMOverlay *overlay = node(Overlay, ast);
    append(NoteTarget(self), overlay);
}

static void visitEmptyLine(void *self, ASTEmptyLine *ast)
{
    __Trace__
}

static void visitMidi(void *self, ASTMidi *ast)
{
    __Trace__

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitMidiParam(void *self, ASTMidiParam *ast)
{
    __Trace__
}

static void visitPropagateAccidental(void *self, ASTPropagateAccidental *ast)
{
    __Trace__
}


Analyzer *ABCASTAnalyzerCreate(ParseContext *context)
{
    ABCASTAnalyzer *self = calloc(1, sizeof(ABCASTAnalyzer));

    self->visitor.visitRoot = visitRoot;
    self->visitor.visitFileIdentification = visitFileIdentification;
    self->visitor.visitStringInformation = visitStringInformation;
    self->visitor.visitTitle = visitTitle;
    self->visitor.visitReferenceNumber = visitReferenceNumber;
    self->visitor.visitKey = visitKey;
    self->visitor.visitKeyParam = visitKeyParam;
    self->visitor.visitMeter = visitMeter;
    self->visitor.visitUnitNoteLength = visitUnitNoteLength;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitTempoParam = visitTempoParam;
    self->visitor.visitParts = visitParts;
    self->visitor.visitInstCharSet = visitInstCharSet;
    self->visitor.visitInstVersion = visitInstVersion;
    self->visitor.visitInstInclude = visitInstInclude;
    self->visitor.visitInstCreator = visitInstCreator;
    self->visitor.visitInstLineBreak = visitInstLineBreak;
    self->visitor.visitInstDecoration = visitInstDecoration;
    self->visitor.visitSymbolLine = visitSymbolLine;
    self->visitor.visitMacro = visitMacro;
    self->visitor.visitContinuation = visitContinuation;
    self->visitor.visitRedefinableSymbol = visitRedefinableSymbol;
    self->visitor.visitVoice = visitVoice;
    self->visitor.visitVoiceParam = visitVoiceParam;
    self->visitor.visitTuneBody = visitTuneBody;
    self->visitor.visitLineBreak = visitLineBreak;
    self->visitor.visitAnnotation = visitAnnotation;
    self->visitor.visitDecoration = visitDecoration;
    self->visitor.visitNote = visitNote;
    self->visitor.visitBrokenRhythm = visitBrokenRhythm;
    self->visitor.visitRest = visitRest;
    self->visitor.visitRepeatBar = visitRepeatBar;
    self->visitor.visitTie = visitTie;
    self->visitor.visitSlur = visitSlur;
    self->visitor.visitDot = visitDot;
    self->visitor.visitGraceNote = visitGraceNote;
    self->visitor.visitTuplet = visitTuplet;
    self->visitor.visitChord = visitChord;
    self->visitor.visitOverlay = visitOverlay;
    self->visitor.visitEmptyLine = visitEmptyLine;
    self->visitor.visitMidi = visitMidi;
    self->visitor.visitMidiParam = visitMidiParam;
    self->visitor.visitPropagateAccidental = visitPropagateAccidental;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    self->context = context;

    regcomp(&self->nthRepeatRegex, "(\\[|\\|:*)[[:digit:]]", REG_EXTENDED);

    return &self->analyzer;
}


static BaseNote KeyChar2BaseNote(char c)
{
    const BaseNote baseNoteTable[] = {
        BaseNote_A, BaseNote_B, BaseNote_C,
        BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
    };

    return baseNoteTable[tolower(c) - 97];
}

static bool parseNoteLength(char *_string, NoteLength *noteLength)
{
    int length = strlen(_string);
    char *string = alloca(length + 2);
    memcpy(string, _string, length);
    string[length] = '$';
    string[length + 1] = '\0';

    int multiplier = 1;
    int divider = 1;
    bool isDivider = false;
    char digits[8];
    int digitCursor = 0;

    char *pc = string;
    char c;
    while ((c = *pc++)) {
        if (isdigit(c)) {
            if (7 <= digitCursor) {
                return false;
            }
            else {
                digits[digitCursor++] = c;
                digits[digitCursor] = '\0';
            }
        }
        else {
            if (0 < digitCursor) {
                if (isDivider) {
                    divider /= 2;
                    divider *= atoi(digits);
                }
                else {
                    multiplier = atoi(digits);
                }

                digitCursor = 0;
            }

            if ('/' == c) {
                divider *= 2;
                isDivider = true;
            }
        }
    }

    noteLength->multiplier = multiplier;
    noteLength->divider = divider;
    return true;
}
