#include "NAMidiParser.h"

#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"

#include <stdlib.h>
#include <ctype.h>
#include <sys/param.h>

#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))
#define isValidRange(v, from, to) (from <= v && v <= to)

int yyparse(void *scanner, const char *filepath, ParserCallback callback, ParserErrorCallback errorCallback);
int yyget_lineno(void *scanner);
int yyget_column(void *scanner);

void yy_flush_buffer(YY_BUFFER_STATE  state, void *scanner);

struct _NAMidiParser {
    NAMidiParserCallbacks *callbacks;
    void *receiver;
};

typedef struct _Context {
    NAMidiParserError error;
    int expectedEndCount;
    NAMidiParser *parser;
} Context;

static bool NAMidiParserCallback(void *context, ParseLocation *location, StatementType type, ...);
static void NAMidiParserErrorCallback(void *context, ParseLocation *location, const char *message);

NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks, void *receiver)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->callbacks = callbacks;
    self->receiver = receiver;
    return self;
}

void NAMidiParserDestroy(NAMidiParser *self)
{
    free(self);
}

bool NAMidiParserExecuteParse(NAMidiParser *self, FILE *fp, const char *filepath)
{
    void *scanner;
    bool ret = false;

    Context context;
    memset(&context, 0, sizeof(Context));
    context.parser = self;

    yylex_init_extra(&context, &scanner);
    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    do {
        if (yyparse(scanner, filepath, NAMidiParserCallback, NAMidiParserErrorCallback)) {
            self->callbacks->onParseError(self->receiver, &context.error);
            yy_flush_buffer(state, scanner);
        }
	} while (!feof(fp));
	
    if (0 < context.expectedEndCount) {
        context.error.kind = NAMidiParserErrorKindEndMissing;
        context.error.location.filepath = filepath;
        context.error.location.line = yyget_lineno(scanner);
        context.error.location.column = yyget_column(scanner);
        self->callbacks->onParseError(self->receiver, &context.error);
        goto ERROR;
    }

    ret = true;

ERROR:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    return ret;
}

typedef bool (*NAMidiParserStatemntParser)(Context *context, ParseLocation *location, StatementType type, va_list argList);
static NAMidiParserStatemntParser statementParserTable[StatementTypeCount] = {NULL};

static bool NAMidiParserCallback(void *_context, ParseLocation *location, StatementType type, ...)
{
    Context *context = _context;
    bool success = false;

    va_list argList;
    va_start(argList, type);

    NAMidiParserStatemntParser parser = statementParserTable[type];
    if (parser) {
        success = parser(context, location, type, argList);
    }
    else {
        printf("parser for statment=%s is not implemented.\n", StatementType2String(type));
        success = true;
    }

    if (!success && !context->error.location.filepath) {
        context->error.location.filepath = location->filepath;;
        context->error.location.line = location->line;
        context->error.location.column = location->column;
    }

    va_end(argList);

    return success;
}

static void NAMidiParserErrorCallback(void *_context, ParseLocation *location, const char *message)
{
    Context *context = _context;

    context->error.kind = NAMidiParserErrorKindSyntaxError;
    context->error.location.filepath = location->filepath;
    context->error.location.line = location->line;
    context->error.location.column = location->column;
}

static void ContextCallbackEvent(Context *context, ParseLocation *location, NAMidiParserEventType type, ...)
{
    NAMidiParserEvent event;
    event.type = type;
    event.location.filepath = location->filepath;
    event.location.line = location->line;
    event.location.column = location->column;

    va_list argList;
    va_start(argList, type);
    context->parser->callbacks->onParseEvent(context->parser->receiver, &event, argList);
    va_end(argList);
}

static bool parseTitle(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    char *title = va_arg(argList, char *);
    ContextCallbackEvent(context, location, NAMidiParserEventTypeTitle, title);
    free(title);
    return true;
}

static bool parseResolution(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int resolution = va_arg(argList, int);
    if (!isValidRange(resolution, 1, 9600)) {
        context->error.kind = NAMidiParserErrorKindInvalidResolution;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeResolution, resolution);
    return true;
}

static bool parseTempo(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    double tempo = va_arg(argList, double);
    if (!isValidRange(tempo, 30.0, 300.0)) {
        context->error.kind = NAMidiParserErrorKindInvalidTempo;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeTempo, tempo);
    return true;
}

static bool parseTimeSign(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int numerator = va_arg(argList, int);
    int denominator = va_arg(argList, int);
    if (1 > numerator || 1 > denominator || !isPowerOf2(denominator)) {
        context->error.kind = NAMidiParserErrorKindInvalidTimeSign;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeTimeSign, numerator, denominator);
    return true;
}

static bool parseMeasure(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int measure = va_arg(argList, int);
    if (!isValidRange(measure, 1, NAMidiParserMeasureMax)) {
        context->error.kind = NAMidiParserErrorKindInvalidMeasure;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeMeasure, measure);
    return true;
}

static bool parseMarker(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    char *marker = va_arg(argList, char *);
    ContextCallbackEvent(context, location, NAMidiParserEventTypeMarker, marker);
    free(marker);
    return true;
}

static bool parsePattern(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    char *identifier = va_arg(argList, char *);
    ContextCallbackEvent(context, location, NAMidiParserEventTypePattern, identifier);
    free(identifier);
    return true;
}

static bool parsePatternDefine(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    ++context->expectedEndCount;
    char *identifier = va_arg(argList, char *);
    ContextCallbackEvent(context, location, NAMidiParserEventTypePatternDefine, identifier);
    free(identifier);
    return true;
}

static bool parseEnd(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    if (0 == context->expectedEndCount) {
        context->error.kind = NAMidiParserErrorKindIllegalEnd;
        return false;
    }

    --context->expectedEndCount;
    ContextCallbackEvent(context, location, NAMidiParserEventTypeEnd);
    return true;
}

static bool parseTrack(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int track = va_arg(argList, int);
    if (!isValidRange(track, 1, 16)) {
        context->error.kind = NAMidiParserErrorKindInvalidTrack;
        return false;
    }

    ++context->expectedEndCount;
    ContextCallbackEvent(context, location, NAMidiParserEventTypeTrack, track);
    return true;
}

static bool parseChannel(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int channel = va_arg(argList, int);
    if (!isValidRange(channel, 1, 16)) {
        context->error.kind = NAMidiParserErrorKindInvalidChannel;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeChannel, channel);
    return true;
}

static bool parseVoice(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int msb = va_arg(argList, int);
    int lsb = va_arg(argList, int);
    int programNo = va_arg(argList, int);

    if (!isValidRange(msb, 0, 127)) {
        context->error.kind = NAMidiParserErrorKindInvalidVoiceMSB;
        return false;
    }

    if (!isValidRange(lsb, 0, 127)) {
        context->error.kind = NAMidiParserErrorKindInvalidVoiceLSB;
        return false;
    }

    if (!isValidRange(programNo, 0, 127)) {
        context->error.kind = NAMidiParserErrorKindInvalidVoiceProgramNo;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeSound, msb, lsb, programNo);
    return true;
}

static bool parseVolume(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int volume = va_arg(argList, int);
    if (!isValidRange(volume, 0, 127)) {
        context->error.kind = NAMidiParserErrorKindInvalidVolume;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeVolume, volume);
    return true;
}

static bool parsePan(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int pan = va_arg(argList, int);
    if (!isValidRange(pan, -64, 64)) {
        context->error.kind = NAMidiParserErrorKindInvalidPan;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypePan, pan);
    return true;
}

static bool parseChorus(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int chorus = va_arg(argList, int);
    if (!isValidRange(chorus, 0, 127)) {
        context->error.kind = NAMidiParserErrorKindInvalidChorus;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeChorus, chorus);
    return true;
}

static bool parseReverb(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int reverb = va_arg(argList, int);
    if (!isValidRange(reverb, 0, 127)) {
        context->error.kind = NAMidiParserErrorKindInvalidReverb;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeReverb, reverb);
    return true;
}

static bool parseTranspose(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int transpose = va_arg(argList, int);
    if (!isValidRange(transpose, -24, 24)) {
        context->error.kind = NAMidiParserErrorKindInvalidTranspose;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeTranspose, transpose);
    return true;
}

static bool parseKey(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    char *keySign = va_arg(argList, char *);
    ContextCallbackEvent(context, location, NAMidiParserEventTypeKeySign, keySign);
    free(keySign);
    return true;
}

static bool parseNote(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    char *noteString = va_arg(argList, char *);
    int step = va_arg(argList, int);
    int gatetime = va_arg(argList, int);
    int velocity = va_arg(argList, int);

    if (!isValidRange(step, -1, 65535)) {
        context->error.kind = NAMidiParserErrorKindInvalidStep;
        return false;
    }

    if (!isValidRange(step, -1, 65535)) {
        context->error.kind = NAMidiParserErrorKindInvalidGatetime;
        return false;
    }

    if (!isValidRange(velocity, -1, 127)) {
        context->error.kind = NAMidiParserErrorKindInvalidVelocity;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeNote, noteString, step, gatetime, velocity);
    free(noteString);
    return true;
}

static bool parseRest(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    int step = va_arg(argList, int);

    if (!isValidRange(step, 0, 65535)) {
        context->error.kind = NAMidiParserErrorKindInvalidStep;
        return false;
    }

    ContextCallbackEvent(context, location, NAMidiParserEventTypeRest, step);
    return true;
}

static bool parseInclude(Context *context, ParseLocation *location, StatementType type, va_list argList)
{
    char filename[256];
    char *_filename = va_arg(argList, char *);
    int len = strlen(_filename);
    strncpy(filename, _filename + 1, len - 2);
    filename[len - 2] = '\0';
    ContextCallbackEvent(context, location, NAMidiParserEventTypeIncludeFile, filename);
    free(_filename);
    return true;
}

static void __attribute__((constructor)) initializeTable()
{
    statementParserTable[StatementTypeTitle] = parseTitle;
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
    statementParserTable[StatementTypeNote] = parseNote;
    statementParserTable[StatementTypeRest] = parseRest;
    statementParserTable[StatementTypeInclude] = parseInclude;
}

const char *NAMidiParserErrorKind2String(NAMidiParserErrorKind kind)
{
#define CASE(kind) case kind: return &(#kind[21])
    switch (kind) {
    CASE(NAMidiParserErrorKindSyntaxError);

    CASE(NAMidiParserErrorKindInvalidResolution);
    CASE(NAMidiParserErrorKindInvalidTempo);
    CASE(NAMidiParserErrorKindInvalidTimeSign);
    CASE(NAMidiParserErrorKindInvalidMeasure);
    CASE(NAMidiParserErrorKindInvalidTrack);
    CASE(NAMidiParserErrorKindInvalidChannel);
    CASE(NAMidiParserErrorKindInvalidVoiceMSB);
    CASE(NAMidiParserErrorKindInvalidVoiceLSB);
    CASE(NAMidiParserErrorKindInvalidVoiceProgramNo);
    CASE(NAMidiParserErrorKindInvalidVolume);
    CASE(NAMidiParserErrorKindInvalidPan);
    CASE(NAMidiParserErrorKindInvalidChorus);
    CASE(NAMidiParserErrorKindInvalidReverb);
    CASE(NAMidiParserErrorKindInvalidTranspose);
    CASE(NAMidiParserErrorKindInvalidNote);
    CASE(NAMidiParserErrorKindInvalidStep);
    CASE(NAMidiParserErrorKindInvalidGatetime);
    CASE(NAMidiParserErrorKindInvalidVelocity);

    CASE(NAMidiParserErrorKindIllegalEnd);
    CASE(NAMidiParserErrorKindEndMissing);

    default:
       break;
    }

    return "Unknown error kind";
#undef CASE
}
