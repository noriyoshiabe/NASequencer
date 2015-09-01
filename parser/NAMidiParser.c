#include "NAMidiParser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include "NAArray.h"
#include "NASet.h"
#include "NAMap.h"
#include "NAStack.h"
#include "NAByteBuffer.h"
#include "NAUtil.h"

#include <stdlib.h>
#include <ctype.h>
#include <libgen.h>

#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))
#define isValidRange(v, from, to) (from <= v && v <= to)

extern int NAMidi_parse(yyscan_t scanner);

typedef struct _StatementHeader {
    StatementType type;
    ParseLocation location;
    int length;
} StatementHeader;

typedef struct _BuildContext {
    NAByteBuffer *buffer;
    NAMap *patternContexts;
    NAArray *patternIdentifiers;
} BuildContext;

static BuildContext *BuildContextCreate()
{
    BuildContext *self = calloc(1, sizeof(BuildContext));
    self->buffer = NAByteBufferCreate(1024);
    self->patternContexts = NAMapCreate(NAHashCString, NADescriptionCString, NULL);
    self->patternIdentifiers = NAArrayCreate(16, NADescriptionCString);
    return self;
}

static void BuildContextDestroy(BuildContext *self)
{
    NAByteBufferDestroy(self->buffer);

    NAMapTraverseValue(self->patternContexts, (void *)BuildContextDestroy);
    NAMapDestroy(self->patternContexts);

    NAArrayTraverse(self->patternIdentifiers, free);
    NAArrayDestroy(self->patternIdentifiers);

    free(self);
}

static bool NAMidiParserParseFile(void *self, const char *filepath);
static void NAMidiParserDestroy(void *self);

static bool NAMidiParserBuildSequence(NAMidiParser *self);

struct _NAMidiParser {
    Parser interface;
    ParseResult *result;
    char *currentFile;
    NASet *fileSet;
    NASet *readingFileSet;
    NAStack *fileStack;
    BuildContext *context;
    NAStack *contextStack;
};

Parser *NAMidiParserCreate(ParseResult *result)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->interface.parseFile = NAMidiParserParseFile;
    self->interface.destroy = NAMidiParserDestroy;
    self->result = result;
    self->fileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->fileStack = NAStackCreate(4);
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->context = BuildContextCreate();
    self->contextStack = NAStackCreate(4);
    return (Parser *)self;
}

static bool NAMidiParserParseFile(void *_self, const char *filepath)
{
    NAMidiParser *self = _self;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        self->result->error.kind = ParseErrorKindFileNotFound;
        return false;
    }

    char *copiedFilePath;
    if (!(copiedFilePath = NASetGet(self->fileSet, (char *)filepath))) {
        copiedFilePath = strdup(filepath);
        NASetAdd(self->fileSet, copiedFilePath);
        NAArrayAppend(self->result->filepaths, copiedFilePath);
    }

    if (self->currentFile) {
        NAStackPush(self->fileStack, self->currentFile);
    }

    self->currentFile = copiedFilePath;
    NASetAdd(self->readingFileSet, copiedFilePath);

    yyscan_t scanner;
    NAMidi_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = NAMidi__create_buffer(fp, YY_BUF_SIZE, scanner);
    NAMidi__switch_to_buffer(state, scanner);

    bool success = 0 == NAMidi_parse(scanner);

    self->currentFile = NAStackPop(self->fileStack);
    NASetRemove(self->readingFileSet, copiedFilePath);
    
    NAMidi__delete_buffer(state, scanner);
    NAMidi_lex_destroy(scanner);
    fclose(fp);

    return success && (self->currentFile ? true : NAMidiParserBuildSequence(self));
}

static void NAMidiParserDestroy(void *_self)
{
    NAMidiParser *self = _self;

    NAStackDestroy(self->fileStack);
    NASetDestroy(self->fileSet);
    NASetDestroy(self->readingFileSet);
    BuildContextDestroy(self->context);
    NAStackDestroy(self->contextStack);
    free(self);
}

bool NAMidiParserProcess(NAMidiParser *self, int line, int column, StatementType type, va_list argList)
{
    bool success = true;

    StatementHeader header;
    header.type = type;
    header.location.line = line;
    header.location.column = column;
    header.location.filepath = self->currentFile;

    switch (type) {
    case StatementTypeResolution:
        {
            int resolution = va_arg(argList, int);
            if (!isValidRange(resolution, 1, 9600)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, resolution);
            }
        }
        break;
    case StatementTypeTitle:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case StatementTypeTempo:
        {
            double tempo = va_arg(argList, double);
            if (!isValidRange(tempo, 30.0, 300.0)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(float);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteFloat(self->context->buffer, tempo);
            }
        }
        break;
    case StatementTypeTimeSign:
        {
            int numerator = va_arg(argList, int);
            int denominator = va_arg(argList, int);
            if (1 > numerator || 1 > denominator || !isPowerOf2(denominator)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int) * 2;
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, numerator);
                NAByteBufferWriteInteger(self->context->buffer, denominator);
            }
        }
        break;
    case StatementTypeMeasure:
        {
            int measure = va_arg(argList, int);
            if (!isValidRange(measure, 1, ParserMeasureMax)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, measure);
            }
        }
        break;
    case StatementTypeMarker:
    case StatementTypePattern:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case StatementTypePatternDefine:
        {
            char *patternIdentifier = strdup(va_arg(argList, char *));

            if (NAMapContainsKey(self->context->patternContexts, patternIdentifier)) {
                NAMidiParserError(self, line, column, ParseErrorKindDuplicatePatternIdentifier);
                free(patternIdentifier);
                success = false;
                break;
            }

            BuildContext *buildContext = BuildContextCreate();
            NAMapPut(self->context->patternContexts, patternIdentifier, buildContext);
            NAArrayAppend(self->context->patternIdentifiers, patternIdentifier);

            NAStackPush(self->contextStack, self->context);
            self->context = buildContext;
        }
        break;
    case StatementTypeEnd:
        {
            BuildContext *buildContext = NAStackPop(self->contextStack);
            if (!buildContext) {
                NAMidiParserError(self, line, column, ParseErrorKindUnexpectedEnd);
                success = false;
                break;
            }

            self->context = buildContext;
        }
        break;
    case StatementTypeTrack:
        {
            int track = va_arg(argList, int);
            if (!isValidRange(track, 1, 16)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, track);
            }
        }
        break;
    case StatementTypeChannel:
        {
            int channel = va_arg(argList, int);
            if (!isValidRange(channel, 1, 16)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, channel);
            }
        }
        break;
    case StatementTypeVoice:
        {
            int msb = va_arg(argList, int);
            int lsb = va_arg(argList, int);
            int programNo = va_arg(argList, int);

            if (!isValidRange(msb, 0, 127)
                    || !isValidRange(lsb, 0, 127)
                    || !isValidRange(programNo, 0, 127)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int) * 3;
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, msb);
                NAByteBufferWriteInteger(self->context->buffer, lsb);
                NAByteBufferWriteInteger(self->context->buffer, programNo);
            }
        }
        break;
    case StatementTypeVolume:
        {
            int volume = va_arg(argList, int);
            if (!isValidRange(volume, 0, 127)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, volume);
            }
        }
        break;
    case StatementTypePan:
        {
            int pan = va_arg(argList, int);
            if (!isValidRange(pan, -64, 64)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, pan);
            }
        }
        break;
    case StatementTypeChorus:
        {
            int chorus = va_arg(argList, int);
            if (!isValidRange(chorus, 0, 127)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, chorus);
            }
        }
        break;
    case StatementTypeReverb:
        {
            int reverb = va_arg(argList, int);
            if (!isValidRange(reverb, 0, 127)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, reverb);
            }
        }
        break;
    case StatementTypeTranspose:
        {
            int transpose = va_arg(argList, int);
            if (!isValidRange(transpose, -64, 64)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, transpose);
            }
        }
        break;
    case StatementTypeKey:
        {
            char *keyString = va_arg(argList, char *);

            char keyChar = tolower(keyString[0]);
            bool sharp = NULL != strchr(keyString, '#');
            bool flat = NULL != strchr(keyString, 'b');
            bool major = NULL == strstr(keyString, "min");

            KeySign keySign = NoteTableGetKeySign(keyChar, sharp, flat, major);
            if (KeySignInvalid == keySign) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, keySign);
            }
        }
        break;
    case StatementTypeNote:
        {
            const BaseNote noteTable[] = {
                BaseNote_A, BaseNote_B, BaseNote_C,
                BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
            };

            char *pc = va_arg(argList, char *);

            BaseNote baseNote = noteTable[tolower(*pc) - 97];
            Accidental accidental = AccidentalNone;

            switch (*(++pc)) {
            case '#':
                accidental = AccidentalSharp;
                ++pc;
                break;
            case 'b':
                accidental = AccidentalFlat;
                ++pc;
                break;
            case 'n':
                accidental = AccidentalNatural;
                ++pc;
                break;
            }

            int octave = atoi(pc);

            int step = va_arg(argList, int);
            int gatetime = va_arg(argList, int);
            int velocity = va_arg(argList, int);

            if (!isValidRange(step, -1, 65535)
                    || !isValidRange(gatetime, -1, 65535)
                    || !isValidRange(velocity, -1, 127)
                    || !isValidRange(octave, -2, 8)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int) * 6;
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, baseNote);
                NAByteBufferWriteInteger(self->context->buffer, accidental);
                NAByteBufferWriteInteger(self->context->buffer, octave);
                NAByteBufferWriteInteger(self->context->buffer, step);
                NAByteBufferWriteInteger(self->context->buffer, gatetime);
                NAByteBufferWriteInteger(self->context->buffer, velocity);
            }
        }
        break;
    case StatementTypeRest:
        {
            int step = va_arg(argList, int);
            if (!isValidRange(step, 0, 65535)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
            }
            else {
                header.length = sizeof(int);
                NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
                NAByteBufferWriteInteger(self->context->buffer, step);
            }
        }
        break;
    case StatementTypeInclude:
        {
            char *filename = va_arg(argList, char *);
            char *directory = dirname((char *)self->currentFile);
            char *fullPath = NAUtilBuildPathWithDirectory(directory, filename);

            if (NASetContains(self->readingFileSet, fullPath)) {
                NAMidiParserError(self, line, column, ParseErrorKindCircularFileInclude);
                free(fullPath);
                success = false;
                break;
            }

            success = NAMidiParserParseFile(self, fullPath);
            free(fullPath);
        }
        break;
    default:
        break;
    }

    return success;
}

void NAMidiParserError(NAMidiParser *self, int line, int column, ParseErrorKind errorKind)
{
    self->result->error.kind = errorKind;
    self->result->error.location.line = line;
    self->result->error.location.column = column;
    self->result->error.location.filepath = self->currentFile;
}

static void BuildContextDump(BuildContext *self, char *name, int indent)
{
    if (name) {
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("-- pattern [%s] buffer --\n", name);
    }
    else {
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("-- song buffer --\n");
    }

    StatementHeader *header;
    void *data;

    while (NAByteBufferReadData(self->buffer, &header, sizeof(StatementHeader))) {
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("statement: %s - %d [%s %d %d]\n",
                StatementType2String(header->type),
                header->length, header->location.filepath, header->location.line, header->location.column);
        NAByteBufferReadData(self->buffer, &data, header->length);
    }

    int count = NAArrayCount(self->patternIdentifiers);
    char **identifiers = NAArrayGetValues(self->patternIdentifiers);
    for (int i = 0; i < count; ++i) {
        BuildContext *context = NAMapGet(self->patternContexts, identifiers[i]);
        BuildContextDump(context, identifiers[i], indent + 2);
    }
}

static bool NAMidiParserBuildSequence(NAMidiParser *self)
{
    BuildContextDump(self->context, NULL, 0);
    // TODO
    self->result->sequence = SequenceCreate();
    return true;
}
