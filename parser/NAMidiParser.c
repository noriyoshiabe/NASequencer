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

typedef struct _Context {
    NAByteBuffer *buffer;
    NAMap *patternMap;
    NAArray *patternIdentifiers;

    int id;
    int track;
    int transpose;
    KeySign keySign;
    struct {
        int channel;
        int tick;
        int gatetime;
        int velocity;
    } tracks[16];
    struct {
        char *pattern;
        char *current;
    } ctx;

    bool shallowCopy;
} Context;

static Context *ContextCreate();
static Context *ContextCreateShallowCopy(Context *self);
static void ContextDestroy(Context *self);
static int ContextGetLength(Context *self);

struct _NAMidiParser {
    Parser interface;
    ParseResult *result;
    char *currentFile;
    NASet *fileSet;
    NASet *readingFileSet;
    NAStack *fileStack;
    Context *context;
    NAStack *contextStack;
};

static bool NAMidiParserParseFile(void *self, const char *filepath);
static void NAMidiParserDestroy(void *self);

static bool NAMidiParserParseFileInternal(NAMidiParser *self, const char *filepath, int line, int column);
static bool NAMidiParserBuildSequence(NAMidiParser *self);
static bool NAMidiParserParseStatement(NAMidiParser *self, Context *context, Sequence *sequence);

Parser *NAMidiParserCreate(ParseResult *result)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->interface.parseFile = NAMidiParserParseFile;
    self->interface.destroy = NAMidiParserDestroy;
    self->result = result;
    self->fileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->fileStack = NAStackCreate(4);
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->context = ContextCreate();
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
    ContextDestroy(self->context);
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
                success = false;
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
                success = false;
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
                success = false;
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
                success = false;
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
            char *context = va_arg(argList, char *);
            
            header.length = strlen(string) + 1;
            if (context) {
                header.length += strlen(context) + 1;
            }

            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
            if (context) {
                NAByteBufferWriteString(self->context->buffer, context);
            }
        }
        break;
    case StatementTypeContext:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case StatementTypeContextDefault:
        {
            char *string = "default";
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case StatementTypeContextEnd:
        {
            header.length = 0;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        }
        break;
    case StatementTypePatternDefine:
        {
            char *patternIdentifier = strdup(va_arg(argList, char *));

            if (NAMapContainsKey(self->context->patternMap, patternIdentifier)) {
                NAMidiParserError(self, line, column, ParseErrorKindDuplicatePatternIdentifier);
                free(patternIdentifier);
                success = false;
                break;
            }

            Context *buildContext = ContextCreate();
            NAMapPut(self->context->patternMap, patternIdentifier, buildContext);
            NAArrayAppend(self->context->patternIdentifiers, patternIdentifier);

            NAStackPush(self->contextStack, self->context);
            self->context = buildContext;
        }
        break;
    case StatementTypeEnd:
        {
            Context *buildContext = NAStackPop(self->contextStack);
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
                success = false;
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
                success = false;
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
                success = false;
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
    case StatementTypeSynth:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case StatementTypeVolume:
        {
            int volume = va_arg(argList, int);
            if (!isValidRange(volume, 0, 127)) {
                NAMidiParserError(self, line, column, ParseErrorKindInvalidValue);
                success = false;
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
                success = false;
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
                success = false;
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
                success = false;
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
                success = false;
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
                success = false;
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
                success = false;
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
                success = false;
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

static bool NAMidiParserBuildSequence(NAMidiParser *self)
{
    Sequence *sequence = SequenceCreate();

    bool success = NAMidiParserParseStatement(self, self->context, sequence);
    TimeTableSetLength(sequence->timeTable, ContextGetLength(self->context));
    SequenceSortEvents(sequence);

    self->result->sequence = sequence;
    return success;
}

static bool NAMidiParserParseStatement(NAMidiParser *self, Context *context, Sequence *sequence)
{
    bool success = true;
    StatementHeader *header;

    int *tick = &context->tracks[context->track].tick;

    while (NAByteBufferReadData(context->buffer, &header, sizeof(StatementHeader))) {
        bool skip = false;

        switch (header->type) {
        case StatementTypeContext:
        case StatementTypeContextDefault:
        case StatementTypeContextEnd:
            break;
        default:
            if (context->ctx.pattern) {
                if (context->ctx.current && 0 != strcmp(context->ctx.pattern, context->ctx.current)) {
                    skip = true;
                }
            }
            else {
                if (context->ctx.current && 0 != strcmp("default", context->ctx.current)) {
                    skip = true;
                }
            }
            break;
        }

        if (skip) {
            void *vp;
            NAByteBufferReadData(context->buffer, &vp, header->length);
            continue;
        }

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
                TempoEvent *event = MidiEventAlloc(MidiEventTypeTempo, ++context->id, *tick, sizeof(TempoEvent) - sizeof(MidiEvent));
                NAByteBufferReadFloat(context->buffer, &event->tempo);
                NAArrayAppend(sequence->events, event);
                TimeTableAddTempo(sequence->timeTable, *tick, event->tempo);
            }
            break;
        case StatementTypeTimeSign:
            {
                TimeEvent *event = MidiEventAlloc(MidiEventTypeTime, ++context->id, *tick, sizeof(TimeEvent) - sizeof(MidiEvent));
                NAByteBufferReadInteger(context->buffer, &event->numerator);
                NAByteBufferReadInteger(context->buffer, &event->denominator);
                NAArrayAppend(sequence->events, event);
                TimeTableAddTimeSign(sequence->timeTable, *tick, (TimeSign){event->numerator, event->denominator});
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
                MarkerEvent *event = MidiEventAlloc(MidiEventTypeMarker, ++context->id, *tick, strlen(marker) + 1);
                strcpy(event->text, marker);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case StatementTypePattern:
            {
                char *patternIdentifier;
                int readLength = NAByteBufferReadString(context->buffer, &patternIdentifier);
                Context *pattern = NAMapGet(context->patternMap, patternIdentifier);
                if (!pattern) {
                    NAMidiParserError(self, header->location.line, header->location.column, ParseErrorKindPatternMissing);
                    success = false;
                    break;
                }

                Context *copy = ContextCreateShallowCopy(context);
                copy->buffer = pattern->buffer;
                copy->patternMap = pattern->patternMap;
                copy->patternIdentifiers = pattern->patternIdentifiers;

                if (readLength < header->length) {
                    NAByteBufferReadString(context->buffer, &copy->ctx.pattern);
                }

                NAByteBufferSeekFirst(copy->buffer);
                copy->ctx.current = NULL;

                success = NAMidiParserParseStatement(self, copy, sequence);

                for (int i = 0; i < 16; ++i) {
                    context->tracks[i].tick = copy->tracks[i].tick;
                }
                context->id = copy->id;

                ContextDestroy(copy);
            }
            break;
        case StatementTypeContext:
        case StatementTypeContextDefault:
            {
                if (context->ctx.current) {
                    NAMidiParserError(self, header->location.line, header->location.column, ParseErrorKindUnexpectedContextStart);
                    success = false;
                }
                else {
                    NAByteBufferReadString(context->buffer, &context->ctx.current);
                    char *c = context->ctx.current;
                    while (*c) {
                        *c = tolower(*c);
                        ++c;
                    }
                }
            }
            break;
        case StatementTypeContextEnd:
            {
                if (!context->ctx.current) {
                    NAMidiParserError(self, header->location.line, header->location.column, ParseErrorKindUnexpectedContextEnd);
                    success = false;
                }
                else {
                    context->ctx.current = NULL;
                }
            }
            break;
        case StatementTypePatternDefine:
        case StatementTypeEnd:
            // never reach
            break;
        case StatementTypeTrack:
            {
                NAByteBufferReadInteger(context->buffer, &context->track);
                tick = &context->tracks[context->track].tick;
            }
            break;
        case StatementTypeChannel:
            {
                NAByteBufferReadInteger(context->buffer, &context->tracks[context->track].channel);
            }
            break;
        case StatementTypeVoice:
            {
                VoiceEvent *event = MidiEventAlloc(MidiEventTypeVoice, ++context->id, *tick, sizeof(VoiceEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->msb);
                NAByteBufferReadInteger(context->buffer, &event->lsb);
                NAByteBufferReadInteger(context->buffer, &event->programNo);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case StatementTypeSynth:
            {
                char *identifier;
                NAByteBufferReadString(context->buffer, &identifier);
                SynthEvent *event = MidiEventAlloc(MidiEventTypeSynth, ++context->id, *tick, strlen(identifier) + 1);
                event->channel = context->tracks[context->track].channel;
                strcpy(event->identifier, identifier);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case StatementTypeVolume:
            {
                VolumeEvent *event = MidiEventAlloc(MidiEventTypeVolume, ++context->id, *tick, sizeof(VolumeEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case StatementTypePan:
            {
                PanEvent *event = MidiEventAlloc(MidiEventTypePan, ++context->id, *tick, sizeof(PanEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case StatementTypeChorus:
            {
                ChorusEvent *event = MidiEventAlloc(MidiEventTypeChorus, ++context->id, *tick, sizeof(ChorusEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case StatementTypeReverb:
            {
                ReverbEvent *event = MidiEventAlloc(MidiEventTypeReverb, ++context->id, *tick, sizeof(ReverbEvent) - sizeof(MidiEvent));
                event->channel = context->tracks[context->track].channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
                NAArrayAppend(sequence->events, event);
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

                int noteNo = NoteTableGetNoteNo(context->keySign, baseNote, accidental, octave);
                if (!isValidRange(noteNo, 0, 127)) {
                    NAMidiParserError(self, header->location.line, header->location.column, ParseErrorKindInvalidNoteRange);
                    success = false;
                    break;
                }

                if (-1 == step) {
                    step = 0;
                }

                if (-1 != gatetime) {
                    context->tracks[context->track].gatetime = gatetime;
                }

                if (-1 != velocity) {
                    context->tracks[context->track].velocity = velocity;
                }

                NoteEvent *event = MidiEventAlloc(MidiEventTypeNote, ++context->id, *tick, sizeof(NoteEvent) - sizeof(MidiEvent));
                event->noteNo = noteNo;
                event->channel = context->tracks[context->track].channel;
                event->gatetime = context->tracks[context->track].gatetime;
                event->velocity = context->tracks[context->track].velocity;

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

        if (!success) {
            break;
        }
    }

    return success;
}


static Context *ContextCreate()
{
    Context *self = calloc(1, sizeof(Context));
    self->buffer = NAByteBufferCreate(1024);
    self->patternMap = NAMapCreate(NAHashCString, NADescriptionCString, NULL);
    self->patternIdentifiers = NAArrayCreate(16, NADescriptionCString);

    for (int i = 0; i < 16; ++i) {
        self->tracks[i].channel = 1;
        self->tracks[i].gatetime = 240;
        self->tracks[i].velocity = 100;
    }

    return self;
}

static Context *ContextCreateShallowCopy(Context *self)
{
    Context *copy = calloc(1, sizeof(Context));
    memcpy(copy, self, sizeof(Context));
    copy->shallowCopy = true;
    return copy;
}

static void ContextDestroy(Context *self)
{
    if (!self->shallowCopy) {
        NAByteBufferDestroy(self->buffer);

        NAMapTraverseValue(self->patternMap, ContextDestroy);
        NAMapDestroy(self->patternMap);

        NAArrayTraverse(self->patternIdentifiers, free);
        NAArrayDestroy(self->patternIdentifiers);
    }

    free(self);
}

static int ContextGetLength(Context *self)
{
    int length = 0;
    for (int i = 0; i < 16; ++i) {
        length = MAX(length, self->tracks[i].tick);
    }
    return length;
}
