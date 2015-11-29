#include "NAMidiASTParser.h"
#include "NAMidiAST.h"
#include "NAMidiSEM.h"
#include "NAStack.h"
#include "NASet.h"
#include "NACString.h"
#include "NAUtil.h"
#include "Parser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>

#define __Trace__ printf("%s - %s\n", __FILE__, __func__);

#define node(type, ast) NAMidiSEMNodeCreate(SEMType##type, &ast->node.location)
#define append(list, sem) NAArrayAppend(list->node.children, sem)
#define isValidRange(v, from, to) (from <= v && v <= to)
#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))

struct _NAMidiASTParser {
    ASTVisitor visitor;
    void *parser;
    NAStack *listStack;
    SEMList *currentList;
    NASet *readingFileSet;
};

extern int NAMidi_parse(yyscan_t scanner, const char *filepath, void **node);

static void visitRoot(void *self, ASTRoot *ast);
static void visitResolution(void *self, ASTResolution *ast);
static void visitTitle(void *self, ASTTitle *ast);
static void visitTempo(void *self, ASTTempo *ast);
static void visitTime(void *self, ASTTime *ast);
static void visitKey(void *self, ASTKey *ast);
static void visitMarker(void *self, ASTMarker *ast);
static void visitChannel(void *self, ASTChannel *ast);
static void visitVoice(void *self, ASTVoice *ast);
static void visitSynth(void *self, ASTSynth *ast);
static void visitVolume(void *self, ASTVolume *ast);
static void visitPan(void *self, ASTPan *ast);
static void visitChorus(void *self, ASTChorus *ast);
static void visitReverb(void *self, ASTReverb *ast);
static void visitTranspose(void *self, ASTTranspose *ast);
static void visitRest(void *self, ASTRest *ast);
static void visitNote(void *self, ASTNote *ast);
static void visitInclude(void *self, ASTInclude *ast);
static void visitPattern(void *self, ASTPattern *ast);
static void visitDefine(void *self, ASTDefine *ast);
static void visitContext(void *self, ASTContext *ast);
static void visitIdentifier(void *self, ASTIdentifier *ast);
static void visitNoteParam(void *self, ASTNoteParam *ast);

NAMidiASTParser *NAMidiASTParserCreate(void *parser)
{
    NAMidiASTParser *self = calloc(1, sizeof(NAMidiASTParser));
    
    self->visitor.visitRoot = visitRoot;
    self->visitor.visitResolution = visitResolution;
    self->visitor.visitTitle = visitTitle;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitTime = visitTime;
    self->visitor.visitKey = visitKey;
    self->visitor.visitMarker = visitMarker;
    self->visitor.visitChannel = visitChannel;
    self->visitor.visitVoice = visitVoice;
    self->visitor.visitSynth = visitSynth;
    self->visitor.visitVolume = visitVolume;
    self->visitor.visitPan = visitPan;
    self->visitor.visitChorus = visitChorus;
    self->visitor.visitReverb = visitReverb;
    self->visitor.visitTranspose = visitTranspose;
    self->visitor.visitRest = visitRest;
    self->visitor.visitNote = visitNote;
    self->visitor.visitInclude = visitInclude;
    self->visitor.visitPattern = visitPattern;
    self->visitor.visitDefine = visitDefine;
    self->visitor.visitContext = visitContext;
    self->visitor.visitIdentifier = visitIdentifier;
    self->visitor.visitNoteParam = visitNoteParam;

    self->parser = parser;
    self->listStack = NAStackCreate(4);
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);

    return self;
}

void NAMidiASTParserDestroy(NAMidiASTParser *self)
{
    NAStackDestroy(self->listStack);
    NASetDestroy(self->readingFileSet);
    free(self);
}

static BaseNote KeyChar2BaseNote(char c)
{
    const BaseNote baseNoteTable[] = {
        BaseNote_A, BaseNote_B, BaseNote_C,
        BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
    };

    return baseNoteTable[tolower(c) - 97];
}


Node *NAMidiASTParserBuildSemantics(NAMidiASTParser *self, Node *node)
{
    SEMList *list = NAMidiSEMNodeCreate(SEMTypeList, NULL);
    self->currentList = list;

    char *directory = dirname((char *)node->location.filepath);
    char *fullPath = NAUtilBuildPathWithDirectory(directory, node->location.filepath);
    NASetAdd(self->readingFileSet, fullPath);
    
    node->accept(node, self);
    
    NASetRemove(self->readingFileSet, fullPath);
    free(fullPath);

    return (Node *)list;
}

static void visitRoot(void *_self, ASTRoot *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitResolution(void *_self, ASTResolution *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->resolution, 1, 9600)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidResolution, NACStringFromInteger(ast->resolution), NULL);
        return;
    }
    
    SEMResolution *sem = node(Resolution, ast);
    sem->resolution = ast->resolution;
    append(self->currentList, sem);
}

static void visitTitle(void *_self, ASTTitle *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    SEMTitle *sem = node(Title, ast);
    sem->title = strdup(ast->title);
    append(self->currentList, sem);
}

static void visitTempo(void *_self, ASTTempo *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->tempo, 30.0, 300.0)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidTempo, NACStringFromFloat(ast->tempo, 2), NULL);
        return;
    }

    SEMTempo *sem = node(Tempo, ast);
    sem->tempo = ast->tempo;
    append(self->currentList, sem);
}

static void visitTime(void *_self, ASTTime *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (1 > ast->numerator || 1 > ast->denominator || !isPowerOf2(ast->denominator)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidTimeSign, NACStringFromInteger(ast->numerator), NACStringFromInteger(ast->denominator), NULL);
        return;
    }

    SEMTime *sem = node(Time, ast);
    sem->numerator = ast->numerator;
    sem->denominator = ast->denominator;
    append(self->currentList, sem);
}

static void visitKey(void *_self, ASTKey *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    BaseNote baseNote = KeyChar2BaseNote(ast->keyString[0]);
    bool sharp = NULL != strchr(&ast->keyString[1], '#');
    bool flat = NULL != strchr(&ast->keyString[1], 'b');
    Mode mode = NULL != strstr(&ast->keyString[1], "min") ? ModeMinor : ModeMajor;

    NoteTable *noteTable = NoteTableCreate(baseNote, sharp, flat, mode);
    if (NoteTableHasUnusualKeySign(noteTable)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidKeySign, ast->keyString, NULL);
        NoteTableRelease(noteTable);
        return;
    }

    SEMKey *sem = node(Key, ast);
    sem->noteTable = noteTable;
    append(self->currentList, sem);
}

static void visitMarker(void *_self, ASTMarker *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    SEMMarker *sem = node(Marker, ast);
    sem->text = strdup(ast->text);
    append(self->currentList, sem);
}

static void visitChannel(void *_self, ASTChannel *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->number, 1, 16)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidChannel, NACStringFromInteger(ast->number), NULL);
        return;
    }

    SEMChannel *sem = node(Channel, ast);
    sem->number = ast->number;
    append(self->currentList, sem);
}

static void visitVoice(void *_self, ASTVoice *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->msb, 0, 127) || !isValidRange(ast->lsb, 0, 127) || !isValidRange(ast->programNo, 0, 127)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidVoice,
                NACStringFromInteger(ast->msb), NACStringFromInteger(ast->lsb), NACStringFromInteger(ast->programNo), NULL);
        return;
    }

    SEMVoice *sem = node(Voice, ast);
    sem->msb = ast->msb;
    sem->lsb = ast->lsb;
    sem->programNo = ast->programNo;
    append(self->currentList, sem);
}

static void visitSynth(void *_self, ASTSynth *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    SEMSynth *sem = node(Synth, ast);
    sem->name = strdup(ast->name);
    append(self->currentList, sem);
}

static void visitVolume(void *_self, ASTVolume *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidVolume, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMVolume *sem = node(Volume, ast);
    sem->value = ast->value;
    append(self->currentList, sem);
}

static void visitPan(void *_self, ASTPan *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->value, -64, 64)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidPan, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMPan *sem = node(Pan, ast);
    sem->value = ast->value;
    append(self->currentList, sem);
}

static void visitChorus(void *_self, ASTChorus *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidChorus, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMChorus *sem = node(Chorus, ast);
    sem->value = ast->value;
    append(self->currentList, sem);
}

static void visitReverb(void *_self, ASTReverb *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidReverb, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMReverb *sem = node(Reverb, ast);
    sem->value = ast->value;
    append(self->currentList, sem);
}

static void visitTranspose(void *_self, ASTTranspose *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->value, -64, 64)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidTranspose, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMTranspose *sem = node(Transpose, ast);
    sem->value = ast->value;
    append(self->currentList, sem);
}

static void visitRest(void *_self, ASTRest *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (!isValidRange(ast->step, 0, 65535)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidStep, NACStringFromInteger(ast->step), NULL);
        return;
    }

    SEMRest *sem = node(Rest, ast);
    sem->step = ast->step;
    append(self->currentList, sem);
}

static void visitNote(void *_self, ASTNote *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    int step = -1;
    int gatetime = -1;
    int velocity = -1;

    int count = NAArrayCount(ast->node.children);
    for (int i = 0; i < count; ++i) {
        ASTNoteParam *np = NAArrayGetValueAt(ast->node.children, i);
        switch (i) {
        case 0:
            step = np->value;
            if (!isValidRange(step, -1, 65535)) {
                ParserError(self->parser, &np->node.location, NAMidiParseErrorInvalidStep, NACStringFromInteger(step), NULL);
                return;
            }
            break;
        case 1:
            gatetime = np->value;
            if (!isValidRange(gatetime, -1, 65535)) {
                ParserError(self->parser, &np->node.location, NAMidiParseErrorInvalidGatetime, NACStringFromInteger(gatetime), NULL);
                return;
            }
            break;
        case 2:
            velocity = np->value;
            if (!isValidRange(gatetime, -1, 127)) {
                ParserError(self->parser, &np->node.location, NAMidiParseErrorInvalidVelocity, NACStringFromInteger(velocity), NULL);
                return;
            }
            break;
        default:
            ParserError(self->parser, &np->node.location, NAMidiParseErrorTooManyNoteParams, NACStringFromInteger(np->value), NULL);
            return;
        }
    }

    char *pc = ast->noteString;

    BaseNote baseNote = KeyChar2BaseNote(*pc);
    Accidental accidental = AccidentalNone;

    int octave = SEMNOTE_OCTAVE_NONE;
    char *c;
    while (*(c = ++pc)) {
        switch (*c) {
        case '#':
            accidental = AccidentalSharp == accidental ? AccidentalDoubleSharp : AccidentalSharp;
            break;
        case 'b':
            accidental = AccidentalFlat == accidental ? AccidentalDoubleFlat : AccidentalFlat;
            break;
        case 'n':
            accidental = AccidentalNatural;
            break;
        default:
            octave = atoi(c);
            break;
        }

        if (SEMNOTE_OCTAVE_NONE != octave) {
            break;
        }
    }

    if (SEMNOTE_OCTAVE_NONE != octave && !isValidRange(octave, -2, 8)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorInvalidOctave, NACStringFromInteger(octave), ast->noteString, NULL);
        return;
    }

    SEMNote *sem = node(Note, ast);
    sem->step = step;
    sem->baseNote = baseNote;
    sem->accidental = accidental;
    sem->octave = octave;
    sem->gatetime = gatetime;
    sem->velocity = velocity;
    sem->noteString = strdup(ast->noteString);
    append(self->currentList, sem);
}

static void visitInclude(void *_self, ASTInclude *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    const char *ext = NAUtilGetFileExtenssion(ast->filepath);
    if (0 != strcmp("namidi", ext)) {
        ParserError(self, &ast->node.location, NAMidiParseErrorUnsupportedFileTypeInclude, ext, ast->filepath, NULL);
        return;
    }

    char *directory = dirname((char *)ast->node.location.filepath);
    char *fullPath = NAUtilBuildPathWithDirectory(directory, ast->filepath);

    if (NASetContains(self->readingFileSet, fullPath)) {
        ParserError(self->parser, &ast->node.location, NAMidiParseErrorCircularFileInclude, ast->filepath, NULL);
        free(fullPath);
        return;
    }

    FILE *fp = fopen(fullPath, "r");
    if (!fp) {
        ParserError(self, &ast->node.location, NAMidiParseErrorIncludeFileNotFound, ast->filepath, NULL);
        free(fullPath);
        return;
    }

    NASetAdd(self->readingFileSet, fullPath);

    yyscan_t scanner;
    NAMidi_lex_init_extra(NULL, &scanner);
    YY_BUFFER_STATE state = NAMidi__create_buffer(fp, YY_BUF_SIZE, scanner);
    NAMidi__switch_to_buffer(state, scanner);

    Node *node = NULL;
    int ret = NAMidi_parse(scanner, fullPath, (void **)&node);
    node->accept(node, self);
    NodeDestroy(node);

    NAMidi__delete_buffer(state, scanner);
    NAMidi_lex_destroy(scanner);

    NASetRemove(self->readingFileSet, fullPath);

    free(fullPath);
    fclose(fp);
}

static void visitPattern(void *_self, ASTPattern *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    SEMPattern *sem = node(Pattern, ast);

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        ASTIdentifier *node = iterator->next(iterator);
        NAArrayAppend(sem->ctxIdList, strdup(node->idString));
    }

    append(self->currentList, sem);
}

static void visitDefine(void *_self, ASTDefine *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    if (NAMapContainsKey(self->currentList->patternMap, ast->identifier)) {
        Node *original = NAMapGet(self->currentList->patternMap, ast->identifier);
        ParserError(self, &ast->node.location, NAMidiParseErrorDuplicatePatternIdentifier,
                ast->identifier, original->location.filepath, NACStringFromInteger(original->location.line), NULL);
        return;
    }

    SEMList *list = node(List, ast);
    list->identifier = strdup(ast->identifier);
    NAMapPut(self->currentList->patternMap, list->identifier, list);

    NAStackPush(self->listStack, self->currentList);
    self->currentList = list;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->currentList = NAStackPop(self->listStack);
}

static void visitContext(void *_self, ASTContext *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    SEMContext *sem = node(Context, ast);

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        ASTIdentifier *node = iterator->next(iterator);
        NAArrayAppend(sem->ctxIdList, strdup(node->idString));
    }

    append(self->currentList, sem);
}

static void visitIdentifier(void *_self, ASTIdentifier *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitNoteParam(void *_self, ASTNoteParam *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}
