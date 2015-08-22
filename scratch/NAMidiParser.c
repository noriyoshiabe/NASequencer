#include "NAMidiParser.h"

#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"
#include "NoteTable.h"

#include "NAMap.h"

#include <stdlib.h>
#include <libgen.h>
#include <limits.h>

#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))
#define isValidRange(v, from, to) (from <= v && v <= to)

int yyparse(void *scanner, const char *filepath, ParserCallback callback, ParserErrorCallback errorCallback);

typedef struct _Statement {
    ParseLocation location;
    StatementType type;
    char *string;
    union {
        int i;
        float f;
    } values[4];
} Statement;

typedef struct _StatementList {
    Statement *array;
    int count;
    int capacity;
} StatementList;

static StatementList *StatementListCreate();
static Statement *StatementListAlloc(StatementList *self);
static void StatementListDestroy(StatementList *self);

typedef enum {
    NAMidiParserStateSong,
    NAMidiParserStatePattern,
} NAMidiParserState;

struct _NAMidiParser {
    NAMidiParserError error;
    StatementList *songStatements;
    char **filepaths;
    int fileCount;
    NAMap *patterns;

    struct {
        NAMidiParserState state;
        StatementList *currentStatements;
        bool inTrack;
    } parseContext;
};

static bool NAMidiParserParseFile(NAMidiParser *self, const char *filepath);
static bool NAMidiParserCallback(void *context, ParseLocation *location, StatementType type, ...);
static void NAMidiParserErrorCallback(void *context, ParseLocation *location, const char *message);

static char *getRealPath(const char *filepath);
static char *buildPathWithDirectory(const char *directory, const char *filepath);

NAMidiParser *NAMidiParserCreate()
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->songStatements = StatementListCreate();

    self->patterns = NAMapCreate(NAHashCString, NADescriptionCString, NULL);
    self->parseContext.state = NAMidiParserStateSong;
    self->parseContext.currentStatements = self->songStatements;

    return self;
}

void NAMidiParserDestroy(NAMidiParser *self)
{
    for (int i = 0; i < self->fileCount; ++i) {
        free(self->filepaths[i]);
    }

    if (self->filepaths) {
        free(self->filepaths);
    }

    uint8_t iteratorBuffer[NAMapIteratorSize];
    NAIterator *iterator = NAMapGetIterator(self->patterns, iteratorBuffer);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        StatementListDestroy(entry->value);
    }
    NAMapDestroy(self->patterns);

    StatementListDestroy(self->songStatements);
    free(self);
}

bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath)
{
    if (!NAMidiParserParseFile(self, filepath)) {
        return false;
    }

    // TODO Render?

    return true;
}
static bool NAMidiParserParseFile(NAMidiParser *self, const char *filepath)
{
    void *scanner;
    bool ret = false;

    self->filepaths = realloc(self->filepaths, (self->fileCount + 2) * sizeof(char *));

    char *_filepath = getRealPath(filepath);

    _filepath = _filepath ? _filepath : strdup(filepath);
    self->filepaths[self->fileCount] = _filepath;
    self->filepaths[++self->fileCount] = NULL;

    FILE *fp = fopen(_filepath, "r");
    if (!fp) {
        self->error.kind = NAMidiParserErrorKindFileNotFound;
        self->error.message = "include file is not found.";
        return ret;
    }

    yylex_init_extra(self, &scanner);

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, _filepath, NAMidiParserCallback, NAMidiParserErrorCallback)) {
        goto ERROR;
    }

    // TODO state check

    ret = true;

ERROR:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    fclose(fp);

    return ret;
}

void NAMidiParserRender(NAMidiParser *self, void *view, NAMidiParserRenderHandler handler)
{
    // TODO remove?
}

const NAMidiParserError *NAMidiParserGetError(NAMidiParser *self)
{
    return (const NAMidiParserError *)&self->error;
}

const char **NAMidiParserGetFilepaths(NAMidiParser *self)
{
    return (const char **)self->filepaths;
}

typedef bool (*NAMidiParserStatemntParser)(NAMidiParser *self, Statement *statement, va_list argList);
static NAMidiParserStatemntParser statementParserTable[StatementTypeCount] = {NULL};

static bool NAMidiParserCallback(void *context, ParseLocation *location, StatementType type, ...)
{
    NAMidiParser *self = context;
    bool success = false;

    va_list argList;
    va_start(argList, type);

    NAMidiParserStatemntParser parser = statementParserTable[type];
    if (parser) {
        printf("statment=%s\n", StatementType2String(type));
        Statement *statement = StatementListAlloc(self->parseContext.currentStatements);
        statement->type = type;
        statement->location = *location;
        success = parser(self, statement, argList);
    }
    else {
        printf("parser for statment=%s is not implemented.\n", StatementType2String(type));
        success = true;
    }

    if (!success && !self->error.filepath) {
        self->error.filepath = location->filepath;;
        self->error.line = location->line;
        self->error.column = location->column;
    }

    va_end(argList);

    return success;
}

static void NAMidiParserErrorCallback(void *context, ParseLocation *location, const char *message)
{
    NAMidiParser *self = context;

    self->error.kind = NAMidiParserErrorKindSyntaxError;
    self->error.message = message;
    self->error.filepath = location->filepath;
    self->error.line = location->line;
    self->error.column = location->column;
}

static StatementList *StatementListCreate()
{
    StatementList *self = calloc(1, sizeof(StatementList));
    self->capacity = 32;
    self->array = calloc(self->capacity, sizeof(Statement));
    return self;
}

static Statement *StatementListAlloc(StatementList *self)
{
    if (self->capacity <= self->count) {
        self->capacity *= 2;
        self->array = realloc(self->array, self->capacity * sizeof(Statement));
    }

    Statement *ret = &self->array[self->count++];
    memset(ret, 0, sizeof(Statement));
    return ret;
}

static void StatementListDestroy(StatementList *self)
{
    for (int i = 0; i < self->count; ++i) {
        if (self->array[i].string) {
            free(self->array[i].string);
        }
    }

    free(self->array);
    free(self);
}

static bool parseResolution(NAMidiParser *self, Statement *statment, va_list argList)
{
    int resolution = va_arg(argList, int);
    if (!isValidRange(resolution, 1, 9600)) {
        self->error.kind = NAMidiParserErrorKindInvalidResolution;
        self->error.message = "invalid range of resolution.";
        return false;
    }

    statment->values[0].i = resolution;
    return true;
}

static bool parseTempo(NAMidiParser *self, Statement *statment, va_list argList)
{
    double tempo = va_arg(argList, double);
    if (!isValidRange(tempo, 30.0, 300.0)) {
        self->error.kind = NAMidiParserErrorKindInvalidTempo;
        self->error.message = "invalid range of tempo.";
        return false;
    }

    statment->values[0].f = tempo;
    return true;
}

static bool parseTimeSign(NAMidiParser *self, Statement *statment, va_list argList)
{
    int numerator = va_arg(argList, int);
    int denominator = va_arg(argList, int);
    if (1 > numerator || 1 > denominator || !isPowerOf2(denominator)) {
        self->error.kind = NAMidiParserErrorKindInvalidTimeSign;
        self->error.message = "invalid time sign.";
        return false;
    }

    statment->values[0].i = numerator;
    statment->values[1].i = denominator;
    return true;
}

static bool parseMeasure(NAMidiParser *self, Statement *statment, va_list argList)
{
    int measure = va_arg(argList, int);
    if (!isValidRange(measure, 1, 9999)) {
        self->error.kind = NAMidiParserErrorKindInvalidMeasure;
        self->error.message = "invalid range of measure.";
        return false;
    }

    statment->values[0].i = measure;
    return true;
}

static bool parseMarker(NAMidiParser *self, Statement *statment, va_list argList)
{
    statment->string = strdup(va_arg(argList, char *));
    return true;
}

static bool parsePattern(NAMidiParser *self, Statement *statment, va_list argList)
{
    statment->string = strdup(va_arg(argList, char *));
    return true;
}

static bool parsePatternDefine(NAMidiParser *self, Statement *statment, va_list argList)
{
    if (NAMidiParserStateSong != self->parseContext.state) {
        self->error.kind = NAMidiParserErrorKindIllegalPatternDefineInPattern;
        self->error.message = "cannnot define pattern inside other pattern.";
        return false;
    }

    if (self->parseContext.inTrack) {
        self->error.kind = NAMidiParserErrorKindIllegalPatternDefineInTrack;
        self->error.message = "cannnot define pattern inside track.";
        return false;
    }

    statment->string = strdup(va_arg(argList, char *));
    self->parseContext.state = NAMidiParserStatePattern;

    StatementList *statmentList = StatementListCreate();
    NAMapPut(self->patterns, statment->string, statmentList);
    self->parseContext.currentStatements = statmentList;
    return true;
}

static bool parseEnd(NAMidiParser *self, Statement *statment, va_list argList)
{
    if (NAMidiParserStatePattern != self->parseContext.state && !self->parseContext.inTrack) {
        self->error.kind = NAMidiParserErrorKindIllegalEnd;
        self->error.message = "end statement with no pattern nor track definition.";
        return false;
    }

    if (self->parseContext.inTrack) {
        self->parseContext.inTrack = false;
    }
    else {
        self->parseContext.currentStatements = self->songStatements;
        self->parseContext.state = NAMidiParserStateSong;
    }

    return true;
}

static bool parseTrack(NAMidiParser *self, Statement *statment, va_list argList)
{
    if (self->parseContext.inTrack) {
        self->error.kind = NAMidiParserErrorKindIllegalTrackStartInTrack;
        self->error.message = "cannnot define track inside other track.";
        return false;
    }

    int track = va_arg(argList, int);
    if (!isValidRange(track, 1, 16)) {
        self->error.kind = NAMidiParserErrorKindInvalidTrack;
        self->error.message = "invalid range of track number.";
        return false;
    }

    statment->values[0].i = track;
    self->parseContext.inTrack = true;
    return true;
}

static bool parseChannel(NAMidiParser *self, Statement *statment, va_list argList)
{
    int channel = va_arg(argList, int);
    if (!isValidRange(channel, 1, 16)) {
        self->error.kind = NAMidiParserErrorKindInvalidChannel;
        self->error.message = "invalid range of channel.";
        return false;
    }

    statment->values[0].i = channel;
    return true;
}

static bool parseVoice(NAMidiParser *self, Statement *statment, va_list argList)
{
    int msb = va_arg(argList, int);
    int lsb = va_arg(argList, int);
    int programNo = va_arg(argList, int);

    if (!isValidRange(msb, 0, 127)) {
        self->error.kind = NAMidiParserErrorKindInvalidVoiceMSB;
        self->error.message = "invalid range of msb.";
        return false;
    }

    if (!isValidRange(lsb, 0, 127)) {
        self->error.kind = NAMidiParserErrorKindInvalidVoiceLSB;
        self->error.message = "invalid range of lsb.";
        return false;
    }

    if (!isValidRange(programNo, 0, 127)) {
        self->error.kind = NAMidiParserErrorKindInvalidVoiceProgramNo;
        self->error.message = "invalid range of program number.";
        return false;
    }

    statment->values[0].i = msb;
    statment->values[1].i = lsb;
    statment->values[2].i = programNo;
    return true;
}

static bool parseVolume(NAMidiParser *self, Statement *statment, va_list argList)
{
    int volume = va_arg(argList, int);
    if (!isValidRange(volume, 0, 127)) {
        self->error.kind = NAMidiParserErrorKindInvalidVolume;
        self->error.message = "invalid range of volume.";
        return false;
    }

    statment->values[0].i = volume;
    return true;
}

static bool parsePan(NAMidiParser *self, Statement *statment, va_list argList)
{
    int pan = va_arg(argList, int);
    if (!isValidRange(pan, -64, 64)) {
        self->error.kind = NAMidiParserErrorKindInvalidPan;
        self->error.message = "invalid range of pan.";
        return false;
    }

    statment->values[0].i = pan;
    return true;
}

static bool parseChorus(NAMidiParser *self, Statement *statment, va_list argList)
{
    int chorus = va_arg(argList, int);
    if (!isValidRange(chorus, 0, 127)) {
        self->error.kind = NAMidiParserErrorKindInvalidChorus;
        self->error.message = "invalid range of chorus.";
        return false;
    }

    statment->values[0].i = chorus;
    return true;
}

static bool parseReverb(NAMidiParser *self, Statement *statment, va_list argList)
{
    int reverb = va_arg(argList, int);
    if (!isValidRange(reverb, 0, 127)) {
        self->error.kind = NAMidiParserErrorKindInvalidReverb;
        self->error.message = "invalid range of reverb.";
        return false;
    }

    statment->values[0].i = reverb;
    return true;
}

static bool parseTranspose(NAMidiParser *self, Statement *statment, va_list argList)
{
    int transpose = va_arg(argList, int);
    if (!isValidRange(transpose, -24, 24)) {
        self->error.kind = NAMidiParserErrorKindInvalidTranspose;
        self->error.message = "invalid range of transpose.";
        return false;
    }

    statment->values[0].i = transpose;
    return true;
}

static bool parseKey(NAMidiParser *self, Statement *statment, va_list argList)
{
    char *keyString = va_arg(argList, char *);
    char keyChar = keyString[0];

    bool sharp = NULL != strchr(keyString, '#');
    bool flat = NULL != strchr(keyString, 'b');
    bool major = NULL == strstr(keyString, "min");

    NoteTableKeySign key = NoteTableGetKeySign(keyChar, sharp, flat, major);
    if (NoteTableKeySignInvalid == key) {
        self->error.kind = NAMidiParserErrorKindInvalidKeySign;
        self->error.message = "invalid key sign.";
        return false;
    }

    statment->values[0].i = key;
    return true;
}

static bool parseInclude(NAMidiParser *self, Statement *statment, va_list argList)
{
    char filename[256];
    char *_filename = va_arg(argList, char *);
    int len = strlen(_filename);
    strncpy(filename, _filename + 1, len - 2);
    filename[len - 2] = '\0';

    char *directory = dirname((char *)statment->location.filepath);
    char *fullPath = buildPathWithDirectory(directory, filename);

    bool ret = NAMidiParserParseFile(self, fullPath);
    free(fullPath);

    return ret;
}

static void __attribute__((constructor)) initializeTable()
{
    statementParserTable[StatementTypeResolution] = parseResolution;
    statementParserTable[StatementTypeTempo] = parseTempo;
    statementParserTable[StatementTypeTimeSign] = parseTimeSign;
    statementParserTable[StatementTypeMeasure] = parseMeasure;
    statementParserTable[StatementTypeMarker] = parseMarker;
    statementParserTable[StatementTypePattern] = parsePattern;
    statementParserTable[StatementTypePatternDefine] = parsePatternDefine;
    statementParserTable[StatementTypeEnd] = parseEnd;
    statementParserTable[StatementTypeTrack] = parseTrack;
    statementParserTable[StatementTypeChannel] = parseChannel;
    statementParserTable[StatementTypeVoice] = parseVoice;
    statementParserTable[StatementTypeVolume] = parseVolume;
    statementParserTable[StatementTypePan] = parsePan;
    statementParserTable[StatementTypeChorus] = parseChorus;
    statementParserTable[StatementTypeReverb] = parseReverb;
    statementParserTable[StatementTypeTranspose] = parseTranspose;
    statementParserTable[StatementTypeKey] = parseKey;
    statementParserTable[StatementTypeInclude] = parseInclude;
}

static char *getRealPath(const char *filepath)
{
    char buf[PATH_MAX];
    char *_filepath = realpath(filepath, buf);
    if (!_filepath) {
        return NULL;
    }

    char *ret = malloc(strlen(_filepath) + 1);
    strcpy(ret, _filepath);
    return ret;
}

static char *buildPathWithDirectory(const char *directory, const char *filename)
{
    char buf[PATH_MAX];
    snprintf(buf, PATH_MAX, "%s/%s", directory, filename);
    char *ret = malloc(strlen(buf) + 1);
    strcpy(ret, buf);
    return ret;
}
