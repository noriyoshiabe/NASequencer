#include "NAMidiParser.h"
#include "Parser.h"

#include <stdio.h>
#include <string.h>

#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))
#define MAKE_ERROR(kind, context, expression, message) ParseErrorCreateWithError(kind, context->filepath, message, expression->location)

static bool __dispatch__INTEGER(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    *((int32_t *)value) = expression->v.i;
    return true;
}

static bool __dispatch__FLOAT(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    *((float *)value) = expression->v.f;
    return true;
}

static bool __dispatch__STRING(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    size_t len = strlen(expression->v.s) - 2;
    char *str = malloc(len);
    strncpy(str, expression->v.s + 1, len);
    str[len] = '\0';
    *((char **)value) = str;
    return true;
}


static int noteNoString2Int(ParseContext *context, const char *noteNoString)
{
    const struct {
        char *name;
        int baseKey;
    } noteMap[] = {
        {"C",  12},
        {"C#", 13}, {"DB", 13},
        {"D",  14},
        {"D#", 15}, {"EB", 15},
        {"E",  16},
        {"F",  17},
        {"F#", 18}, {"GB", 18},
        {"G",  19},
        {"G#", 20}, {"AB", 20},
        {"A",  21},
        {"A#", 22}, {"BB", 22},
        {"B",  23},
    };

    char octaveStr[4] = {0};
    char *pOctave = octaveStr;

    int tmpOctave = 0;

    char noteNo[8];
    strcpy(noteNo, noteNoString);
    char *pNoteNo = noteNo;
    while (*pNoteNo) {
        *pNoteNo = toupper(*pNoteNo);
        if (*pNoteNo == '-' || *pNoteNo == '+' || isdigit(*pNoteNo)) {
            *pOctave = *pNoteNo;
            *pNoteNo = '\0';
            *(++pOctave) = '\0';
        }
        else if (*pNoteNo == '<') {
            --tmpOctave;
            *pNoteNo = '\0';
        }
        else if (*pNoteNo == '>') {
            ++tmpOctave;
            *pNoteNo = '\0';
        }
        ++pNoteNo;
    }

    int baseKey = -1;
    for (int i = 0; i < sizeof(noteMap)/sizeof(noteMap[0]); ++i) {
        if (0 == strcmp(noteMap[i].name, noteNo)) {
            baseKey = noteMap[i].baseKey;
            break;
        }
    }

    if (-1 == baseKey) {
        return -1;
    }

    int octave;
    if (0 != tmpOctave) {
        octave = context->octave + tmpOctave;
    }
    else {
        octave = '\0' != octaveStr[0] ? atoi(octaveStr) : context->octave;
        context->octave = octave;
    }

    return baseKey + 12 * octave;
}

static bool __dispatch__NOTE_NO(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int noteNo = noteNoString2Int(context, expression->v.s);

    if (-1 == noteNo) {
        *error = MAKE_ERROR(PARSE_ERROR_INVALID_NOTE_NO, expression, "invalid note expression.");
        return false;
    }

    if (127 < noteNo) {
        *error = MAKE_ERROR(PARSE_ERROR_INVALID_NOTE_NO, expression, "invalid range of note no.");
        return false;
    }
    
    ParseContext *nbContext = value;

    uint8_t channel = context->channel;
    uint8_t velocity = context->velocity;

    int32_t gatetime = 0 < context->gatetime ? context->gatetime : context->gatetime + nbContext->step;
    gatetime = MAX(0, gatetime);

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        bool success = true;

        switch (expr->tokenType) {
        case VELOCITY:
            success = parseExpression(expr, context, &velocity, error);
            break;
        case GATETIME:
            success = parseExpression(expr, context, &gatetime, error);
            break;
        }

        if (!success) {
            return false;
        }
    }

    SequenceAddNoteEvent(nbContext->sequence, nbContext->tick, channel, velocity, gatetime);
 
    return true;
}

static bool __dispatch__LOCATION(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int32_t tick = -1;

    if (strrchr(expression->v.s, ':')) {
        char *saveptr;
        char buf[16];
        int32_t numbers[3];
        int count = 0;
        char *str = buf;
        char *token;

        strcpy(str, expression->v.s);

        while ((token = strtok_r(str, ":", &saveptr))) {
            numbers[count] = atoi(token);
            ++count;
            str = NULL;
        }

        switch (count) {
        case 2:
            tick = SequenceLocation2Tick(context->sequence, 1, numbers[0], numbers[1]);
            break;
        case 3:
            tick = SequenceLocation2Tick(context->sequence, numbers[0], numbers[1], numbers[2]);
            break;
        }
    }
    else if (('b' == expression->v.s[0])) {
        int32_t beat = atoi(&expression->v.s[1]);
        tick = SequenceLocation2Tick(context->sequence, 1, beat, 0);
    }
    else if (('m' == expression->v.s[0])) {
        int32_t measure = atoi(&expression->v.s[1]);
        tick = SequenceLocation2Tick(context->sequence, measure, 1, 0);
    }

    if (-1 == tick) {
        printf("Unexpected location. location=%s", expression->v.s);
        abort();
    }

    *((int32_t *)value) = tick;
    return true;
}

static bool __dispatch__MB_LENGTH(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    char str[16];
    char *pch;

    int32_t length = -1;

    int32_t offset = *((int32_t *)context->option);
    strcpy(str, expression->v.s);

    if ((pch = strrchr(str, 'm'))) {
        *pch = '\0';
        length = SequenceMBLength2Tick(context->sequence, offset, atoi(str), 0);
    }
    else if ((pch = strrchr(str, 'b'))) {
        *pch = '\0';
        length = SequenceMBLength2Tick(context->sequence, offset, 0, atoi(str));
    }

    if (-1 == length) {
        printf("Unexpected location. location=%s", expression->v.s);
        abort();
    }

    *((int32_t *)value) = length;
    return true;
}

static bool __dispatch__QUANTIZE(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    size_t len = strlen(expression->v.s);
    char *str = alloca(len + 1);
    strcpy(str, expression->v.s);

    bool dot = false;
    bool triplet = false;

    switch (str[len - 1]) {
    case '.':
        dot = true;
        str[len - 1] = '\0';
        break;
    case '3':
        triplet = true;
        str[len - 1] = '\0';
        break;
    }

    int denominator = atoi(str + 2);
    if (!isPowerOf2(denominator)) {
        *error = MAKE_ERROR(PARSE_ERROR_INVALID_QUANTIZE, expression, "quantize denominator is not power of 2");
        return false;
    }

    int32_t step;
    if (1 == denominator) {
        step = SequenceMBLength2Tick(context->sequence, 0, 1, 0);
    }
    else {
        step = SequenceGetResolution(context->sequence) * 4 / denominator;
    }

    if (dot) {
        step += step / 2;
    }

    if (triplet) {
        step = step * 2 / 3;
    }

    *((int32_t *)value) = step;
    return true;
}

static bool __dispatch__RESOLUTION(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    if (0 != context->resolution) {
        *error = MAKE_ERROR(PARSE_ERROR_RESOLUTION_REDEFINED, expression, "resolution cannot be defined twice.");
        return false;
    }

    if (!parseExpression(expression->left, context, &context->resolution, error)) {
        return false;
    }

    if (1 > context->resolution) {
        *error = MAKE_ERROR(PARSE_ERROR_INVALID_RESOLUTION, expression, "resolution is invalid.");
        return false;
    }

    SequenceSetResolution(context->sequence, context->resolution);
    return true;
}

static bool __dispatch__TITLE(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    if (NULL != context->title) {
        *error = MAKE_ERROR(error, PARSE_ERROR_TITLE_REDEFINED, expression, "title cannot be defined twice.");
        return false;
    }

    if (!parseExpression(expression->left, context, &context->title, error)) {
        return false;
    }

    SequenceSetTitle(context->sequence, context->title);
    return true;
}

// TODO from here
static bool __dispatch__TIME(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    TimeEvent *event = NATypeNew(TimeEvent, context->tick);

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        if (!parseExpression(expr, context, event, error)) {
            NARelease(event);
            return false;
        }
    }

    TimeTableAddTimeEvent(context->timeTable, event);
    NARelease(event);

    return true;
}

static bool __dispatch__TEMPO(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    TempoEvent *event = NATypeNew(TempoEvent, context->tick);

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        bool success = true;
        int32_t i;

        switch (expr->tokenType) {
        case FLOAT:
            success = parseExpression(expr, context, &event->tempo, error);
            break;
        case INTEGER:
            success = parseExpression(expr, context, &i, error);
            event->tempo = i;
            break;
        default:
            success = parseExpression(expr, context, event, error);
            break;
        }

        if (!success) {
            NARelease(event);
            return false;
        }
    }

    bool ret = true;

    if (30.0 <= event->tempo && event->tempo <= 300.0) {
        TimeTableAddTempoEvent(context->timeTable, event);
    }
    else {
        MAKE_ERROR(error, PARSE_ERROR_INVALID_TEMPO, expression, "invalid range of tempo.");
        ret = false;
    }

    NARelease(event);

    return ret;
}

// TODO obsolete FROM on Parser.y
static bool __dispatch__MARKER(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    char *text = NULL;

    if (!parseExpression(expr, context, &text, error)) {
        return false;
    }

    SequenceAddMarkerEvent(context->sequence, context->tick, text);
    free(text);
    return true;
}

static bool __dispatch__CHANNEL(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    if (expression->parent) {
        switch (expression->parent->tokenType) {
        case REPLACE:
        case MIX:
            return parseExpression(expression->left, context, value, error);
        }
    }
    
    if (!parseExpression(expression->left, context, &context->channel, error)) {
        return false;
    }

    if (context->channel < 1 || 16 < context->channel) {
        MAKE_ERROR(error, PARSE_ERROR_INVALID_CHANNEL, expression, "invalid range of channel.");
        return false;
    }

    return true;
}

static bool __dispatch__VELOCITY(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int32_t val; 

    if (!parseExpression(expression->left, context, &val, error)) {
        return false;
    }

    if (val < 0 || 127 < val) {
        MAKE_ERROR(error, PARSE_ERROR_INVALID_VELOCITY, expression, "invalid range of velocity.");
        return false;
    }

    int tokenType = expression->parent ? expression->parent->tokenType : -1;
    switch (tokenType) {
    case NOTE_NO:
        *((uint8_t *)value) = val;
        break;
    default:
        context->velocity = val;
        break;
    }

    return true;
}

static bool __dispatch__GATETIME(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int32_t val; 

    if (!parseExpression(expression->left, context, &val, error)) {
        return false;
    }

    int tokenType = expression->parent ? expression->parent->tokenType : -1;
    switch (tokenType) {
    case NOTE_NO:
        *((int32_t *)value) = val;
        break;
    default:
        context->gatetime = val;
        break;
    }

    return true;
}

static bool __dispatch__OCTAVE(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    if (!parseExpression(expression->left, context, &context->octave, error)) {
        return false;
    }

    if (context->octave < -2 || 8 < context->octave) {
        MAKE_ERROR(error, PARSE_ERROR_INVALID_OCTAVE, expression, "invalid range of octave.");
        return false;
    }

    return true;
}

static bool __dispatch__NOTE(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int32_t step = -1;

    Expression *noteBlockExpr = NULL;

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        bool success = true;

        switch (expr->tokenType) {
        case FROM:
            success = parseExpression(expr, context, &context->tick, error);
            break;
        case STEP:
            if (-1 != step) {
                MAKE_ERROR(error, PARSE_ERROR_NOTE_BLOCK_STEP_REDEFINED, expression, "step for note block cannot be specified twice.");
                return false;
            }
            success = parseExpression(expr, context, &step, error);
            break;
        case NOTE_BLOCK:
            noteBlockExpr = expr;
            break;
        }

        if (!success) {
            return false;
        }
    }

    if (!noteBlockExpr) {
        MAKE_ERROR(error, PARSE_ERROR_NOTE_BLOCK_MISSING, expression, "note block is missing.");
        return false;
    }

    if (-1 == step) {
        MAKE_ERROR(error, PARSE_ERROR_NOTE_BLOCK_STEP_MISSING, expression, "step for note block is not specified.");
        return false;
    }

    if (1 > step) {
        MAKE_ERROR(error, PARSE_ERROR_INVALID_STEP, expression, "step is invalid.");
        return false;
    }

    return parseExpression(noteBlockExpr, context, &step, error);
}


static bool __dispatch__STEP(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    return parseExpression(expression->left, context, value, error);
}

static bool __dispatch__FROM(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    return parseExpression(expression->left, context, value, error);
}

static bool __dispatch__TO(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    return parseExpression(expression->left, context, value, error);
}

static bool __dispatch__MIX_OR_REPLACE(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int32_t from = 0;
    int32_t to = INT32_MAX;
    int32_t channel = -1;

    Expression *patternBlock = NULL;

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        bool success = true;

        switch (expr->tokenType) {
        case FROM:
            success = parseExpression(expr, context, &from, error);
            break;
        case TO:
            success = parseExpression(expr, context, &to, error);
            break;
        case CHANNEL:
            success = parseExpression(expr, context, &channel, error);
            context->channel = channel;
            break;
        case PATTERN_BLOCK:
            patternBlock = expr;
            break;
        }

        if (!success) {
            return false;
        }
    }

    if (REPLACE == expression->tokenType) {
        CFIndex count = CFArrayGetCount(context->events);
        for (CFIndex i = count - 1; 0 <= i; --i) {
            MidiEvent *event = (MidiEvent *)CFArrayGetValueAtIndex(context->events, i);
            if (from <= event->tick && event->tick < to) {
                if (-1 == channel
                        || (NATypeOf(event, NoteEvent) && channel == ((NoteEvent *)event)->channel)
                        || (NATypeOf(event, SoundSelectEvent) && channel == ((SoundSelectEvent *)event)->channel)) {
                    CFArrayRemoveValueAtIndex(context->events, i);
                }
            }
        }
    }

    ParseContext *local = ParseContextCreateLocal(context);

    if (!parseExpression(patternBlock, local, NULL, error)) {
        NARelease(local);
        return false;
    }

    CFIndex count = CFArrayGetCount(local->events);
    for (int i = 0; i < count; ++i) {
        const MidiEvent *event = CFArrayGetValueAtIndex(local->events, i);
        if (event->tick + from < to) {
            MidiEvent *copied = NACopy(event);
            copied->tick += from;
            ContextAddEvent(context, copied);
            NARelease(copied);
        }
        else {
            break;
        }
    }

    NARelease(local);

    return true;
}

static bool __dispatch__REPLACE(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    return __dispatch__MIX_OR_REPLACE(expression, context, value, error);
}

static bool __dispatch__MIX(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    return __dispatch__MIX_OR_REPLACE(expression, context, value, error);
}

static bool __dispatch__OFFSET(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    return parseExpression(expression->left, context, value, error);
}

static bool __dispatch__LENGTH(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    return parseExpression(expression->left, context, value, error);
}


static bool __dispatch__REST(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    return true;
}

static bool __dispatch__TIE(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    NoteBlockContext *nbContext = value;
    int32_t tick = -1;
    CFIndex count = CFArrayGetCount(nbContext->events);
    for (CFIndex i = count - 1; 0 <= i; --i) {
        NoteEvent *event = (NoteEvent *)CFArrayGetValueAtIndex(nbContext->events, i);
        if (-1 != tick && tick != event->__.tick) {
            break;
        }
        event->gatetime += nbContext->step;
        tick = event->__.tick;
    }

    if (-1 == tick) {
        MAKE_ERROR(error, PARSE_ERROR_INVALID_TIE, expression, "target note for tie is not found.");
        return false;
    }

    return true;
}


static bool __dispatch__IDENTIFIER(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    char *lower = strdup(expression->v.s);
    for (char *p = lower; *p; ++p) *p = tolower(*p);
    *((CFStringRef *)value) = CFStringCreateWithCString(NULL, lower, kCFStringEncodingUTF8);
    free(lower);
    return true;
}


static bool __dispatch__TIME_SIGN(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    TimeEvent *timeEvent = value;

    if (!parseExpression(expression->left, context, &timeEvent->numerator, error)) {
        return false;
    }

    if (timeEvent->numerator < 0 || 64 < timeEvent->numerator ) {
        MAKE_ERROR(error, PARSE_ERROR_INVALID_TIME_SIGN, expression, "invalid range of time sign numerator");
        return false;
    }

    if (!parseExpression(expression->left->right, context, &timeEvent->denominator, error)) {
        return false;
    }

    if (!isPowerOf2(timeEvent->denominator) || 64 < timeEvent->denominator) {
        MAKE_ERROR(error, PARSE_ERROR_INVALID_TIME_SIGN, expression, "time sign denominator is not power of 2");
        return false;
    }

    return true;
}

static bool __dispatch__SOUND_SELECT(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    bool ret = false;

    SoundSelectEvent *event = NATypeNew(SoundSelectEvent, context->tick);
    event->channel = context->channel;

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        if (INTEGER_LIST == expr->tokenType) {
            int32_t integerList[3];
            if (!parseExpression(expr, context, integerList, error)) {
                goto ERROR;
            }

            struct {
                uint8_t *result;
                ParseErrorKind errorKind;
                const char *message;
            } table[] = {
                {&event->msb, PARSE_ERROR_INVALID_MSB, "invalid range of msb"},
                {&event->lsb, PARSE_ERROR_INVALID_LSB, "invalid range of lsb"},
                {&event->programNo, PARSE_ERROR_INVALID_PROGRAM_NO, "invalid range of program no"},
            };

            for (int i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
                if (integerList[i] < 0 || 127 < integerList[i]) {
                    MAKE_ERROR(error, table[i].errorKind, expression, table[i].message);
                    goto ERROR;
                }

                *table[i].result = integerList[i];
            }
        }
        else {
            if (!parseExpression(expr, context, event, error)) {
                goto ERROR;
            }
        }
    }

    ContextAddEvent(context, event);
    ret = true;

ERROR:
    NARelease(event);

    return ret;
}

static bool __dispatch__INTEGER_LIST(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int32_t *integer = value;

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        *integer = expr->v.i;
        ++integer;
    }

    return true;
}

static bool __dispatch__GATETIME_CUTOFF(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    if (!parseExpression(expression->left, context, &context->gatetime, error)) {
        return false;
    }

    context->gatetime *= -1;
    return true;
}

static bool __dispatch__NOTE_LIST(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int32_t from = context->tick;
    int32_t tick = from;

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        if (!parseExpression(expr, context, NULL, error)) {
            return false;
        }

        tick = MAX(tick, context->tick);
        context->tick = from;
    }

    context->tick = tick;
    return true;
}

static bool __dispatch__NOTE_BLOCK(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int32_t step = *((int32_t *)value);
    NoteBlockContext *nbContext = NATypeNew(NoteBlockContext, context->tick, step);

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        if (!parseExpression(expr, context, nbContext, error)) {
            NARelease(nbContext);
            return false;
        }

        nbContext->tick += step;
    }

    context->tick = nbContext->tick;
    context->length = MAX(context->tick, context->length);

    CFArrayAppendArray(context->events, nbContext->events, CFRangeMake(0, CFArrayGetCount(nbContext->events)));

    NARelease(nbContext);

    return true;
}

static bool __dispatch__NOTE_NO_LIST(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    for (Expression *expr = expression->left; expr; expr = expr->right) {
        if (!parseExpression(expr, context, value, error)) {
            return false;
        }
    }

    return true;
}

static bool __dispatch__PATTERN_DEFINE(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    bool ret = false;

    CFStringRef identifier = NULL;
    ParseContext *local = ParseContextCreateLocal(context);

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        switch (expr->tokenType) {
        case IDENTIFIER:
            if (!parseExpression(expr, local, &identifier, error)) {
                goto ERROR;
            }
            break;
        case PATTERN_BLOCK:
        case PATTERN_EXPAND:
            if (!parseExpression(expr, local, NULL, error)) {
                goto ERROR;
            }
            break;
        }
    }

    Sequence *pattern = NATypeNew(Sequence);
    SequenceSetTitle(pattern, identifier);
    SequenceSetTimeTable(pattern, local->timeTable);
    SequenceAddEvents(pattern, local->events);
    pattern->length = local->length;
    
    CFDictionarySetValue(context->patterns, identifier, pattern);
    NARelease(pattern);

    ret = true;

ERROR:
    if (identifier) {
        CFRelease(identifier);
    }

    NARelease(local);

    return ret;
}

static bool __dispatch__PATTERN_BLOCK(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    for (Expression *expr = expression->left; expr; expr = expr->right) {
        if (!parseExpression(expr, context, NULL, error)) {
            return false;
        }
    }

    return true;
}

static bool __dispatch__PATTERN_EXPAND(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    bool ret = false;

    CFStringRef identifier = NULL;
    CFIndex count;
    int32_t from = context->tick;
    int32_t offset = 0;
    int32_t expandLength = -1;

    Expression *patternExtendBlockExpr = NULL;
    Expression *lengthExpr = NULL;

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        bool success = true;

        switch (expr->tokenType) {
        case IDENTIFIER:
            success = parseExpression(expr, context, &identifier, error);
            break;
        case FROM:
            success = parseExpression(expr, context, &from, error);
            break;
        case PATTERN_EXTEND_BLOCK:
            patternExtendBlockExpr = expr;
            break;
        case OFFSET:
            context->option = &offset;
            success = parseExpression(expr, context, &offset, error);
            break;
        case LENGTH:
            lengthExpr = expr;
            break;
        }

        if (!success) {
            goto ERROR_1;
        }
    }

    const Sequence *pattern = CFDictionaryGetValue(context->patterns, identifier);
    if (!pattern) {
        size_t len = CFStringGetLength(identifier) + 64;
        char *buf = alloca(len);
        snprintf(buf, len, "pattern named '%s' is missing.", NACFString2CString(identifier));
        MAKE_ERROR(error, PARSE_ERROR_PATTERN_MISSING, expression, buf);
        return ret;
    }

    ParseContext *local = ParseContextCreateLocal(context);

    count = CFArrayGetCount(pattern->timeTable->timeEvents);
    for (int i = 0; i < count; ++i) {
        TimeTableAddTimeEvent(local->timeTable, (TimeEvent *)CFArrayGetValueAtIndex(pattern->timeTable->timeEvents, i));
    }

    count = CFArrayGetCount(pattern->timeTable->tempoEvents);
    for (int i = 0; i < count; ++i) {
        TimeTableAddTempoEvent(local->timeTable, (TempoEvent *)CFArrayGetValueAtIndex(pattern->timeTable->tempoEvents, i));
    }

    if (lengthExpr) {
        local->option = &offset;
        if (!parseExpression(lengthExpr, local, &expandLength, error)) {
            goto ERROR_2;
        }
    }

    count = CFArrayGetCount(pattern->events);
    for (int i = 0; i < count; ++i) {
        const MidiEvent *event = CFArrayGetValueAtIndex(pattern->events, i);
        if (offset <= event->tick && (-1 == expandLength || event->tick < offset + expandLength)) {
            MidiEvent *copied = NACopy(event);
            copied->tick -= offset;
            ContextAddEvent(local, copied);
            NARelease(copied);
        }
    }

    if (patternExtendBlockExpr) {
        if (!parseExpression(patternExtendBlockExpr, local, NULL, error)) {
            goto ERROR_2;
        }
    }

    count = CFArrayGetCount(local->events);
    for (int i = 0; i < count; ++i) {
        const MidiEvent *event = CFArrayGetValueAtIndex(local->events, i);
        if (-1 == expandLength || event->tick < expandLength) {
            MidiEvent *copied = NACopy(event);
            copied->tick += from;
            ContextAddEvent(context, copied);
            NARelease(copied);
        }
    }

    int32_t length = (-1 != expandLength ? expandLength : pattern->length) - offset;
    context->tick += length;
    context->length = MAX(context->tick, from + length);

    ret = true;

ERROR_2:
    NARelease(local);

ERROR_1:
    if (identifier) {
        CFRelease(identifier);
    }

    return ret;
}

static bool __dispatch__PATTERN_EXPAND_LIST(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    int32_t from = context->tick;
    int32_t tick = from;

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        if (!parseExpression(expr, context, NULL, error)) {
            return false;
        }

        tick = MAX(tick, context->tick);
        context->tick = from;
    }

    context->tick = tick;
    return true;
}

static bool __dispatch__PATTERN_EXTEND_BLOCK(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
    for (Expression *expr = expression->left; expr; expr = expr->right) {
        if (!parseExpression(expr, context, value, error)) {
            return false;
        }
    }

    return true;
}


#define SET_FUNCTION(type) registerExpressionParser(type, __dispatch__##type)

static void __attribute__((constructor)) initializeTable()
{
    SET_FUNCTION(INTEGER);
    SET_FUNCTION(FLOAT);
    SET_FUNCTION(STRING);

    SET_FUNCTION(NOTE_NO);
    SET_FUNCTION(LOCATION);
    SET_FUNCTION(MB_LENGTH);
    SET_FUNCTION(QUANTIZE);

    SET_FUNCTION(RESOLUTION);
    SET_FUNCTION(TITLE);
    SET_FUNCTION(TIME);
    SET_FUNCTION(TEMPO);
    SET_FUNCTION(MARKER);
    SET_FUNCTION(CHANNEL);
    SET_FUNCTION(VELOCITY);
    SET_FUNCTION(GATETIME);
    SET_FUNCTION(OCTAVE);
    SET_FUNCTION(NOTE);

    SET_FUNCTION(STEP);
    SET_FUNCTION(FROM);
    SET_FUNCTION(TO);
    SET_FUNCTION(REPLACE);
    SET_FUNCTION(MIX);
    SET_FUNCTION(OFFSET);
    SET_FUNCTION(LENGTH);

    SET_FUNCTION(REST);
    SET_FUNCTION(TIE);

    SET_FUNCTION(IDENTIFIER);

    SET_FUNCTION(TIME_SIGN);
    SET_FUNCTION(SOUND_SELECT);
    SET_FUNCTION(INTEGER_LIST);
    SET_FUNCTION(GATETIME_CUTOFF);
    SET_FUNCTION(NOTE_LIST);
    SET_FUNCTION(NOTE_BLOCK);
    SET_FUNCTION(NOTE_NO_LIST);
    SET_FUNCTION(PATTERN_DEFINE);
    SET_FUNCTION(PATTERN_BLOCK);
    SET_FUNCTION(PATTERN_EXPAND);
    SET_FUNCTION(PATTERN_EXPAND_LIST);
    SET_FUNCTION(PATTERN_EXTEND_BLOCK);
}
