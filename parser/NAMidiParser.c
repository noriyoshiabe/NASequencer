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

#define OCTAVE_NONE -99

extern int NAMidi_parse(yyscan_t scanner);

typedef struct _StatementHeader {
    NAMidiStatementType type;
    ParseLocation location;
    int length;
} StatementHeader;

typedef struct _Context {
    NAByteBuffer *buffer;
    NAMap *patternMap;
    NAArray *patternIdentifiers;

    int id;
    int channel;
    int transpose;
    KeySign keySign;
    struct {
        int tick;
        int gatetime;
        int velocity;
        int octave;
    } channels[16];
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

bool NAMidiParserProcess(NAMidiParser *self, int line, int column, NAMidiStatementType type, va_list argList)
{
    bool success = true;

    StatementHeader header;
    header.type = type;
    header.location.line = line;
    header.location.column = column;
    header.location.filepath = self->currentFile;

    switch (type) {
    case NAMidiStatementTypeResolution:
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
    case NAMidiStatementTypeTitle:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case NAMidiStatementTypeTempo:
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
    case NAMidiStatementTypeTimeSign:
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
    case NAMidiStatementTypeMarker:
    case NAMidiStatementTypePattern:
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
    case NAMidiStatementTypeContext:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case NAMidiStatementTypeContextDefault:
        {
            char *string = "default";
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case NAMidiStatementTypeContextEnd:
        {
            header.length = 0;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        }
        break;
    case NAMidiStatementTypePatternDefine:
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
    case NAMidiStatementTypeEnd:
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
    case NAMidiStatementTypeChannel:
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
    case NAMidiStatementTypeVoice:
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
    case NAMidiStatementTypeSynth:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case NAMidiStatementTypeVolume:
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
    case NAMidiStatementTypePan:
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
    case NAMidiStatementTypeChorus:
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
    case NAMidiStatementTypeReverb:
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
    case NAMidiStatementTypeTranspose:
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
    case NAMidiStatementTypeKey:
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
    case NAMidiStatementTypeNote:
        {
            const BaseNote noteTable[] = {
                BaseNote_A, BaseNote_B, BaseNote_C,
                BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
            };

            char *pc = va_arg(argList, char *);

            BaseNote baseNote = noteTable[tolower(*pc) - 97];
            Accidental accidental = AccidentalNone;

            int octave = OCTAVE_NONE;
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

                if (OCTAVE_NONE != octave) {
                    break;
                }
            }

            int step = va_arg(argList, int);
            int gatetime = va_arg(argList, int);
            int velocity = va_arg(argList, int);

            if (!isValidRange(step, -1, 65535)
                    || !isValidRange(gatetime, -1, 65535)
                    || !isValidRange(velocity, -1, 127)
                    || (OCTAVE_NONE != octave && !isValidRange(octave, -2, 8))) {
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
    case NAMidiStatementTypeRest:
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
    case NAMidiStatementTypeInclude:
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

    int *tick = &context->channels[context->channel].tick;

    while (NAByteBufferReadData(context->buffer, &header, sizeof(StatementHeader))) {
        bool skip = false;

        switch (header->type) {
        case NAMidiStatementTypeContext:
        case NAMidiStatementTypeContextDefault:
        case NAMidiStatementTypeContextEnd:
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
        case NAMidiStatementTypeResolution:
            {
                int resolution;
                NAByteBufferReadInteger(context->buffer, &resolution);
                TimeTableSetResolution(sequence->timeTable, resolution);
            }
            break;
        case NAMidiStatementTypeTitle:
            {
                char *title;
                NAByteBufferReadString(context->buffer, &title);
                free(sequence->title);
                sequence->title = strdup(title);
            }
            break;
        case NAMidiStatementTypeTempo:
            {
                TempoEvent *event = MidiEventAlloc(MidiEventTypeTempo, ++context->id, *tick, sizeof(TempoEvent) - sizeof(MidiEvent));
                NAByteBufferReadFloat(context->buffer, &event->tempo);
                NAArrayAppend(sequence->events, event);
                TimeTableAddTempo(sequence->timeTable, *tick, event->tempo);
            }
            break;
        case NAMidiStatementTypeTimeSign:
            {
                TimeEvent *event = MidiEventAlloc(MidiEventTypeTime, ++context->id, *tick, sizeof(TimeEvent) - sizeof(MidiEvent));
                NAByteBufferReadInteger(context->buffer, &event->numerator);
                NAByteBufferReadInteger(context->buffer, &event->denominator);
                NAArrayAppend(sequence->events, event);
                TimeTableAddTimeSign(sequence->timeTable, *tick, (TimeSign){event->numerator, event->denominator});
            }
            break;
        case NAMidiStatementTypeMarker:
            {
                char *marker;
                NAByteBufferReadString(context->buffer, &marker);
                MarkerEvent *event = MidiEventAlloc(MidiEventTypeMarker, ++context->id, *tick, strlen(marker) + 1);
                strcpy(event->text, marker);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case NAMidiStatementTypePattern:
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
                    context->channels[i].tick = copy->channels[i].tick;
                }
                context->id = copy->id;

                ContextDestroy(copy);
            }
            break;
        case NAMidiStatementTypeContext:
        case NAMidiStatementTypeContextDefault:
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
        case NAMidiStatementTypeContextEnd:
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
        case NAMidiStatementTypePatternDefine:
        case NAMidiStatementTypeEnd:
            // never reach
            break;
        case NAMidiStatementTypeChannel:
            {
                NAByteBufferReadInteger(context->buffer, &context->channel);
                tick = &context->channels[context->channel].tick;
            }
            break;
        case NAMidiStatementTypeVoice:
            {
                VoiceEvent *event = MidiEventAlloc(MidiEventTypeVoice, ++context->id, *tick, sizeof(VoiceEvent) - sizeof(MidiEvent));
                event->channel = context->channel;
                NAByteBufferReadInteger(context->buffer, &event->msb);
                NAByteBufferReadInteger(context->buffer, &event->lsb);
                NAByteBufferReadInteger(context->buffer, &event->programNo);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case NAMidiStatementTypeSynth:
            {
                char *identifier;
                NAByteBufferReadString(context->buffer, &identifier);
                SynthEvent *event = MidiEventAlloc(MidiEventTypeSynth, ++context->id, *tick, strlen(identifier) + 1);
                event->channel = context->channel;
                strcpy(event->identifier, identifier);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case NAMidiStatementTypeVolume:
            {
                VolumeEvent *event = MidiEventAlloc(MidiEventTypeVolume, ++context->id, *tick, sizeof(VolumeEvent) - sizeof(MidiEvent));
                event->channel = context->channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case NAMidiStatementTypePan:
            {
                PanEvent *event = MidiEventAlloc(MidiEventTypePan, ++context->id, *tick, sizeof(PanEvent) - sizeof(MidiEvent));
                event->channel = context->channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case NAMidiStatementTypeChorus:
            {
                ChorusEvent *event = MidiEventAlloc(MidiEventTypeChorus, ++context->id, *tick, sizeof(ChorusEvent) - sizeof(MidiEvent));
                event->channel = context->channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case NAMidiStatementTypeReverb:
            {
                ReverbEvent *event = MidiEventAlloc(MidiEventTypeReverb, ++context->id, *tick, sizeof(ReverbEvent) - sizeof(MidiEvent));
                event->channel = context->channel;
                NAByteBufferReadInteger(context->buffer, &event->value);
                NAArrayAppend(sequence->events, event);
            }
            break;
        case NAMidiStatementTypeTranspose:
            {
                NAByteBufferReadInteger(context->buffer, &context->transpose);
            }
            break;
        case NAMidiStatementTypeKey:
            {
                NAByteBufferReadInteger(context->buffer, &context->keySign);
            }
            break;
        case NAMidiStatementTypeNote:
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

                if (OCTAVE_NONE == octave) {
                    octave = context->channels[context->channel].octave;
                }
                else {
                    context->channels[context->channel].octave = octave;
                }

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
                    context->channels[context->channel].gatetime = gatetime;
                }

                if (-1 != velocity) {
                    context->channels[context->channel].velocity = velocity;
                }

                NoteEvent *event = MidiEventAlloc(MidiEventTypeNote, ++context->id, *tick, sizeof(NoteEvent) - sizeof(MidiEvent));
                event->noteNo = noteNo;
                event->channel = context->channel;
                event->gatetime = context->channels[context->channel].gatetime;
                event->velocity = context->channels[context->channel].velocity;

                NAArrayAppend(sequence->events, event);

                *tick += step;
            }
            break;
        case NAMidiStatementTypeRest:
            {
                int step;
                NAByteBufferReadInteger(context->buffer, &step);
                *tick += step;
            }
            break;
        case NAMidiStatementTypeInclude:
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
        self->channels[i].gatetime = 240;
        self->channels[i].velocity = 100;
        self->channels[i].octave = 2;
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
        length = MAX(length, self->channels[i].tick);
    }
    return length;
}
