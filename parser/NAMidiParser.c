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
#include <sys/param.h>

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

    Sequence *sequence;

    int track;
    int transpose;
    KeySign keySign;
    struct {
        int channel;
        int tick;
        int gatetime;
        int velocity;
    } tracks[16];

    bool shallowCopy;
} BuildContext;

static BuildContext *BuildContextCreate()
{
    BuildContext *self = calloc(1, sizeof(BuildContext));
    self->buffer = NAByteBufferCreate(1024);
    self->patternContexts = NAMapCreate(NAHashCString, NADescriptionCString, NULL);
    self->patternIdentifiers = NAArrayCreate(16, NADescriptionCString);
    return self;
}

static BuildContext *BuildContextCreateShallowCopy(BuildContext *self)
{
    BuildContext *copy = calloc(1, sizeof(BuildContext));
    memcpy(copy, self, sizeof(BuildContext));
    copy->shallowCopy = true;
    return copy;
}

static void BuildContextDestroy(BuildContext *self)
{
    if (!self->shallowCopy) {
        NAByteBufferDestroy(self->buffer);

        NAMapTraverseValue(self->patternContexts, (void *)BuildContextDestroy);
        NAMapDestroy(self->patternContexts);

        NAArrayTraverse(self->patternIdentifiers, free);
        NAArrayDestroy(self->patternIdentifiers);
    }

    free(self);
}

static int BuildContextGetLength(BuildContext *self)
{
    int length = 0;
    for (int i = 0; i < 16; ++i) {
        length = MAX(length, self->tracks[i].tick);
    }
    return length;
}

static bool NAMidiParserParseFile(void *self, const char *filepath);
static void NAMidiParserDestroy(void *self);

static bool NAMidiParserParseFileInternal(NAMidiParser *self, const char *filepath, int line, int column);
static bool NAMidiParserBuildSequence(NAMidiParser *self);
static bool NAMidiParserParseBuildContext(NAMidiParser *self, BuildContext *context, char *name);
static bool NAMidiParserParseStatement(NAMidiParser *self, BuildContext *context);

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

static bool NAMidiParserParseFile(void *self, const char *filepath)
{
    return NAMidiParserParseFileInternal(self, filepath, 0, 0);
}

static bool NAMidiParserParseFileInternal(NAMidiParser *self, const char *filepath, int line, int column)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        NAMidiParserError(self, line, column, ParseErrorKindFileNotFound);
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
            
            if (0 != strcmp("namidi", NAUtilGetFileExtenssion(fullPath))) {
                NAMidiParserError(self, line, column, ParseErrorKindUnsupportedFileType);
                free(fullPath);
                success = false;
                break;
            }

            if (NASetContains(self->readingFileSet, fullPath)) {
                NAMidiParserError(self, line, column, ParseErrorKindCircularFileInclude);
                free(fullPath);
                success = false;
                break;
            }

            success = NAMidiParserParseFileInternal(self, fullPath, line, column);
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

    NAByteBufferSeekFirst(self->buffer);
}

static bool NAMidiParserBuildSequence(NAMidiParser *self)
{
#if 1
    BuildContextDump(self->context, NULL, 0);
#endif

    bool success = NAMidiParserParseBuildContext(self, self->context, "Song");
    self->result->sequence = self->context->sequence;
#if 1
    SequenceDescription(self->result->sequence, stdout);
#endif
    return success;
}

static bool NAMidiParserParseBuildContext(NAMidiParser *self, BuildContext *context, char *name)
{
    bool success = true;

    Sequence *sequence = SequenceCreate();
    sequence->title = strdup(name);

    context->sequence = sequence;

    int count = NAArrayCount(context->patternIdentifiers);
    char **identifiers = NAArrayGetValues(context->patternIdentifiers);
    for (int i = 0; i < count; ++i) {
        BuildContext *patternContext = NAMapGet(context->patternContexts, identifiers[i]);
        success = NAMidiParserParseBuildContext(self, patternContext, identifiers[i]);
        if (!success) {
            return false;
        }

        TimeTableSetLength(patternContext->sequence->timeTable, BuildContextGetLength(patternContext));
        SequenceSortEvents(patternContext->sequence);
        NAArrayAppend(sequence->children, patternContext->sequence);
    }

    success = NAMidiParserParseStatement(self, context);
    TimeTableSetLength(sequence->timeTable, BuildContextGetLength(context));
    SequenceSortEvents(sequence);
    return success;
}

static bool NAMidiParserParseStatement(NAMidiParser *self, BuildContext *context)
{
    bool success = true;
    StatementHeader *header;

    Sequence *sequence = context->sequence;
    int *tick = &context->tracks[context->track].tick;

    while (NAByteBufferReadData(context->buffer, &header, sizeof(StatementHeader))) {
        switch (header->type) {
        case StatementTypeResolution:
            {
                int resolution;
                NAByteBufferReadInteger(context->buffer, &resolution);
                TimeTableSetResolution(sequence->timeTable, resolution);
            }
            break;
        case StatementTypeTitle:
            {
                char *title;
                NAByteBufferReadString(context->buffer, &title);
                free(sequence->title);
                sequence->title = strdup(title);
            }
            break;
        case StatementTypeTempo:
            {
                float tempo;
                NAByteBufferReadFloat(context->buffer, &tempo);
                TimeTableAddTempo(sequence->timeTable, *tick, tempo);
            }
            break;
        case StatementTypeTimeSign:
            {
                int numerator, denominator;
                NAByteBufferReadInteger(context->buffer, &numerator);
                NAByteBufferReadInteger(context->buffer, &denominator);
                TimeTableAddTimeSign(sequence->timeTable, *tick, (TimeSign){numerator, denominator});
            }
            break;
        case StatementTypeMeasure:
            {
                int measure;
                NAByteBufferReadInteger(context->buffer, &measure);
                *tick = TimeTableTickByMeasure(sequence->timeTable, measure);
            }
            break;
        case StatementTypeMarker:
            {
                char *marker;
                NAByteBufferReadString(context->buffer, &marker);
                MarkerEvent *event = MidiEventAlloc(MidiEventTypeMarker, *tick, strlen(marker) + 1);
                strcpy(event->text, marker);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case StatementTypePattern:
            {
                char *pattern;
                NAByteBufferReadString(context->buffer, &pattern);
                BuildContext *patternContext = NAMapGet(context->patternContexts, pattern);
                if (!patternContext) {
                    NAMidiParserError(self, header->location.line, header->location.column, ParseErrorKindPatternMissing);
                    success = false;
                    break;
                }

                BuildContext *copy = BuildContextCreateShallowCopy(context);
                copy->buffer = patternContext->buffer;
                copy->patternContexts = patternContext->patternContexts;
                copy->patternIdentifiers = patternContext->patternIdentifiers;
                NAByteBufferSeekFirst(copy->buffer);

                success = NAMidiParserParseStatement(self, copy);
                BuildContextDestroy(copy);
            }
            break;
        case StatementTypePatternDefine:
        case StatementTypeEnd:
            // never reach
            break;
        case StatementTypeTrack:
            {
                NAByteBufferReadInteger(context->buffer, &context->track);
            }
            break;
        case StatementTypeChannel:
            {
                NAByteBufferReadInteger(context->buffer, &context->tracks[context->track].channel);
            }
            break;
        case StatementTypeVoice:
            {
                VoiceEvent *event = MidiEventAlloc(MidiEventTypeVoice, *tick, sizeof(VoiceEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->msb);
                NAByteBufferReadInteger(context->buffer, &event->lsb);
                NAByteBufferReadInteger(context->buffer, &event->programNo);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case StatementTypeVolume:
            {
                VolumeEvent *event = MidiEventAlloc(MidiEventTypeVolume, *tick, sizeof(VolumeEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
            }
            break;
        case StatementTypePan:
            {
                PanEvent *event = MidiEventAlloc(MidiEventTypePan, *tick, sizeof(PanEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
            }
            break;
        case StatementTypeChorus:
            {
                ChorusEvent *event = MidiEventAlloc(MidiEventTypeChorus, *tick, sizeof(ChorusEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
            }
            break;
        case StatementTypeReverb:
            {
                ReverbEvent *event = MidiEventAlloc(MidiEventTypeReverb, *tick, sizeof(ReverbEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
            }
            break;
        case StatementTypeTranspose:
            {
                NAByteBufferReadInteger(context->buffer, &context->transpose);
            }
            break;
        case StatementTypeKey:
            {
                NAByteBufferReadInteger(context->buffer, &context->keySign);
            }
            break;
        case StatementTypeNote:
            {
                BaseNote baseNote;
                Accidental accidental;
                int octave, step, gatetime, velocity;

                NAByteBufferReadInteger(context->buffer, (int *)&baseNote);
                NAByteBufferReadInteger(context->buffer, (int *)&accidental);
                NAByteBufferReadInteger(context->buffer, &octave);
                NAByteBufferReadInteger(context->buffer, &step);
                NAByteBufferReadInteger(context->buffer, &gatetime);
                NAByteBufferReadInteger(context->buffer, &velocity);

                int noteNo = 0;//NoteTableGetNoteNo(context->keySign, baseNote, accidental, octave);
                if (!isValidRange(noteNo, 0, 127)) {
                    NAMidiParserError(self, header->location.line, header->location.column, ParseErrorKindInvalidNoteRange);
                    success = false;
                    break;
                }

                if (-1 == step) {
                    step = 0;
                }

                if (-1 == gatetime) {
                    gatetime = context->tracks[context->track].gatetime;
                }
                else {
                    context->tracks[context->track].gatetime = gatetime;
                }

                if (-1 == velocity) {
                    velocity = context->tracks[context->track].velocity;
                }
                else {
                    context->tracks[context->track].velocity = velocity;
                }

                NoteEvent *event = MidiEventAlloc(MidiEventTypeNote, *tick, sizeof(NoteEvent) - sizeof(MidiEvent));
                event->noteNo = noteNo;
                event->channel = context->tracks[context->track].channel;
                event->gatetime = gatetime;
                event->velocity = velocity;

                NAArrayAppend(sequence->events, event);

                *tick += step;
            }
            break;
        case StatementTypeRest:
            {
                int step;
                NAByteBufferReadInteger(context->buffer, &step);
                *tick += step;
            }
            break;
        case StatementTypeInclude:
            // never reach
            break;
        default:
            // never reach
            break;
        }
    }

    return success;
}
