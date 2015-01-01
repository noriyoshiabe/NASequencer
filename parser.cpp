#include "parser.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cctype>
#include <string>
#include "namidi.tab.h"

extern "C" {

extern void location_init();
extern int yyparse();
extern FILE *yyin;
extern FILE *yyout;
extern int (*__interpret)(int action, int modifier, void *arg);
extern void (*__on_yy_error)(const char *message, int last_line, int last_column);
extern int last_line;
extern int last_column;

}

ParseContext *Parser::_context = NULL;

int Parser::interpret(int action, int modifier, void *arg)
{
    return _context->interpret(action, modifier, arg);
}

void Parser::onYyError(const char *message, int last_line, int last_column)
{
    _context->push_error("%s.", message);
}

ParseContext *Parser::parse(const char *source)
{
    __interpret = Parser::interpret;
    __on_yy_error = Parser::onYyError;
    yyin = fopen(source, "r");
    yyout = fopen("/dev/null", "r");
    location_init();

    ParseContext *context = new ParseContext();

    _context = context;
    if(yyparse()) {
        context->push_error("Parse failed.");
    }
    _context = NULL;

    fclose(yyin);

    return context;
}

void ParseContext::push_error(const char *format, ...)
{
    char buf[256];
    char info[128];

    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    sprintf(info, " line=%d, column=%d", last_line, last_column);
    strcat(buf, info);

    errors.push_back(new std::string(buf));
}

void ParseContext::push_warning(const char *format, ...)
{
    char buf[256];
    char info[128];

    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    sprintf(info, " line=%d, column=%d", last_line, last_column);
    strcat(buf, info);

    warnings.push_back(new std::string(buf));
}

const char* ParseContext::e2Key(int e)
{
#define _S(sym) #sym
    switch (e) {
    case STATEMENT: return _S(STATEMENT);
    case RESOLUTION: return _S(RESOLUTION);
    case SET: return _S(SET);
    case UNSET: return _S(UNSET);
    case TEMPO: return _S(TEMPO);
    case TRACK: return _S(TRACK);
    case TRACK_END: return _S(TRACK_END);
    case TIME_SIGNATURE: return _S(TIME_SIGNATURE);
    case BANK_SELECT: return _S(BANK_SELECT);
    case PROGRAM_CHANGE: return _S(PROGRAM_CHANGE);
    case MARKER: return _S(MARKER);
    case INCLUDE: return _S(INCLUDE);
    case NOTE: return _S(NOTE);
    case ASSIGN: return _S(ASSIGN);
    case DEFAULT: return _S(DEFAULT);
    case NUMERATOR: return _S(NUMERATOR);
    case DENOMINATOR: return _S(DENOMINATOR);
    case VELOCITY: return _S(VELOCITY);
    case GATETIME: return _S(GATETIME);
    case STEP: return _S(STEP);
    case CHANNEL: return _S(CHANNEL);
    case MSB: return _S(MSB);
    case LSB: return _S(LSB);
    case M_B_TICK: return _S(M_B_TICK);
    case B_TICK: return _S(B_TICK);
    case NAME: return _S(NAME);
    case NOTE_NO_LIST: return _S(NOTE_NO_LIST);
    }

    throw Parser::Exception(std::string("Unknown yacc enum: ") + std::to_string(e));
#undef _S
}

const char* ParseContext::e2Keyword(int e)
{
#define _S(sym) #sym
    switch (e) {
    case STATEMENT: return "statemnt";
    case RESOLUTION: return "resolution";
    case SET: return "set";
    case UNSET: return "unset";
    case TEMPO: return "tempo";
    case TRACK: return "track";
    case TRACK_END: return "track_end";
    case TIME_SIGNATURE: return "time_signature";
    case BANK_SELECT: return "bank_select";
    case PROGRAM_CHANGE: return "program_change";
    case MARKER: return "marker";
    case INCLUDE: return "include";
    case NOTE: return "note";
    case ASSIGN: return "assign";
    case DEFAULT: return "default";
    case NUMERATOR:
    case DENOMINATOR: return "time_signature";
    case VELOCITY: return "velocity";
    case GATETIME: return "gatetime";
    case STEP: return "step";
    case CHANNEL: return "channel";
    case MSB: return "msb";
    case LSB: return "lsb";
    case M_B_TICK: return "m_b_tick";
    case B_TICK: return "b_tick";
    case NAME: return "name";
    case NOTE_NO_LIST: return "note no list";
    }

    throw Parser::Exception(std::string("Unknown yacc enum: ") + std::to_string(e));
#undef _S
}



int ParseContext::interpret(int action, int modifier, void *arg)
{
#if 1
    printf("%s %s\n", e2Key(action), e2Key(modifier));
#endif

    switch (action) {
    case STATEMENT:
        switch (modifier) {
        case RESOLUTION:
            resolution();
            break;
        case SET:
            set();
            break;
        case UNSET:
            unset();
            break;
        case TEMPO:
            tempo();
            break;
        case TRACK:
            track();
            break;
        case TRACK_END:
            trackEnd();
            break;
        case TIME_SIGNATURE:
            timeSignature();
            break;
        case BANK_SELECT:
            bankSelect();
            break;
        case PROGRAM_CHANGE:
            programChange();
            break;
        case MARKER:
            marker();
            break;
        case INCLUDE:
            // TBD
            break;
        case NOTE:
            note();
            break;
        }

        updateLastValues(modifier);
        break;

    case ASSIGN:
        assign(modifier, arg);
        break;
    }

    return 0;
}

Value *ParseContext::getValue(int e)
{
    Value *val;
    if ((val = find(localValues, e))) {
        return val;
    }
    if ((val = find(defaultValues, e))) {
        return val;
    }
    if ((val = find(lastValues, e))) {
        return val;
    }

    return NULL;
}

uint32_t ParseContext::calcTick()
{
    Value *val = getValue(STEP);
    uint32_t step = NULL != val ? val->i : 0;
    uint32_t ret = currentTick;
    currentTick += step;
    return ret;
}

Value *ParseContext::find(std::unordered_map<int, Value *> &map, int key)
{
    try {
        return map.at(key);
    } catch (std::out_of_range &e) {
        return NULL;
    }
}

void ParseContext::insert(std::unordered_map<int, Value *> &map, int key, Value *newVal)
{
    removeIf(map, key);
    map.emplace(key, newVal);
}

void ParseContext::removeIf(std::unordered_map<int, Value *> &map, int key)
{
    Value *val = find(map, key);
    if (val) {
        delete val;
        map.erase(key);
    }
}

void ParseContext::assign(int modifier, void *arg)
{
#define _C(type, vp) *((type *)vp)

    Value *val = new Value();

    switch (modifier) {
    case RESOLUTION:
    case DEFAULT:
    case PROGRAM_CHANGE:
    case NUMERATOR:
    case DENOMINATOR:
    case VELOCITY:
    case GATETIME:
    case STEP:
    case CHANNEL:
    case MSB:
    case LSB:
        val->i = _C(int, arg);
        break;
    case TEMPO:
        val->f = _C(float, arg);
        break;
    case NAME:
    case NOTE_NO_LIST:
        val->s = _C(char *, arg);
        break;
    case M_B_TICK:
        val->s = _C(char *, arg);
        val->i = parseMBTick(val->s, true);
        modifier = STEP;
        break;
    case B_TICK:
        val->s = _C(char *, arg);
        val->i = parseMBTick(val->s, false);
        modifier = STEP;
        break;
    }

    insert(localValues, modifier, val);
#undef _C
}

bool ParseContext::canBeLastValue(int type)
{
    switch (type) {
    case PROGRAM_CHANGE:
    case VELOCITY:
    case GATETIME:
    case STEP:
    case CHANNEL:
    case MSB:
    case LSB:
    case NOTE_NO_LIST:
    case NUMERATOR:
    case DENOMINATOR:
        return true;
    }
    return false;
}

void ParseContext::updateLastValues(int statement)
{
    std::vector<int> uselessValues;

    for (std::pair<int, Value *> entry : localValues) {
        if (isUselessValue(statement, entry.first)) {
            uselessValues.push_back(entry.first);
        }

        if (canBeLastValue(entry.first)) {
            insert(lastValues, entry.first, entry.second);
        } else {
            delete entry.second;
        }
    }

    localValues.clear();

    for (int type : uselessValues) {
        push_warning("[%s] is useless for [%s] statement.", e2Key(type), e2Key(statement));
    }
}

bool ParseContext::isUselessValue(int statement, int assign)
{
    switch(statement) {
    case RESOLUTION:
        switch (assign) {
        case RESOLUTION:
            break;
        default:
            return true;
        }
        break;
    case SET:
    case UNSET:
        break;
    case TEMPO:
        switch (assign) {
        case TEMPO:
        case STEP:
            break;
        default:
            return true;
        }
        break;
    case TRACK:
        switch (assign) {
        case NAME:
        case CHANNEL:
        case STEP:
            break;
        default:
            return true;
        }
        break;
    case TRACK_END:
        switch (assign) {
        case STEP:
            break;
        default:
            return true;
        }
        break;
    case TIME_SIGNATURE:
        switch (assign) {
        case STEP:
        case NUMERATOR:
        case DENOMINATOR:
            break;
        default:
            return true;
        }
        break;
    case BANK_SELECT:
        switch (assign) {
        case STEP:
        case CHANNEL:
        case MSB:
        case LSB:
            break;
        default:
            return true;
        }
        break;
    case PROGRAM_CHANGE:
        switch (assign) {
        case STEP:
        case CHANNEL:
        case PROGRAM_CHANGE:
            break;
        default:
            return true;
        }
        break;
    case MARKER:
        switch (assign) {
        case STEP:
        case NAME:
            break;
        default:
            return true;
        }
        break;
    case INCLUDE:
        // TBD
        break;
    case NOTE:
        switch (assign) {
        case STEP:
        case CHANNEL:
        case NOTE_NO_LIST:
        case VELOCITY:
        case GATETIME:
            break;
        default:
            return true;
        }
        break;
    }

    return false;
}

int ParseContext::parseMBTick(char *str, bool includeMeasure)
{
    int step = 0;

    Value *_vn = getValue(NUMERATOR);
    Value *_vd = getValue(DENOMINATOR);

    if (!_vn || !_vd) {
        push_error("Set step with location but time signature is not set.");
        return step;
    }

    char *token;

    int denominator = _vd->i;

    token = strtok(str, ":");

    if (includeMeasure) {
        int numerator = _vn->i;
        int m = atoi(token);
        step += m * sequence->resolution * 4 * numerator / denominator;
        token = strtok(NULL, ":");
    }

    step += atoi(token) * sequence->resolution * 4 / denominator;
    token = strtok(NULL, ":");
    step += atoi(token);

    return step;
}

void ParseContext::resolution()
{
    Value *val = find(localValues, RESOLUTION);
    if (checkValue(val, RESOLUTION)) {
        sequence->resolution = val->i;
    }
}

void ParseContext::set()
{
    Value *val = find(localValues, DEFAULT);
    if (val) {
        for (std::pair<int, Value *> entry : localValues) {
            removeIf(defaultValues, entry.first);
            insert(defaultValues, entry.first, entry.second);
        }

        localValues.clear();
    }
}

void ParseContext::unset()
{
    Value *val = find(localValues, DEFAULT);
    if (val) {
        for (std::pair<int, Value *> entry : localValues) {
            removeIf(defaultValues, entry.first);
            delete entry.second;
        }
    }
    else {
        for (std::pair<int, Value *> entry : localValues) {
            removeIf(lastValues, entry.first);
            delete entry.second;
        }
    }

    localValues.clear();
}

void ParseContext::tempo()
{
    if (!checkCurrentTrack()) {
        return;
    }

    Value *val = getValue(TEMPO);
    if (checkValue(val, TEMPO)) {
        //currentTrack->events.push_back(new TempoEvent(calcTick(), val->f));
    }
}

void ParseContext::track()
{
    currentTick = 0;
    currentTrack = new Track();
    sequence->tracks.push_back(currentTrack);

    Value *val = getValue(NAME);
    if (val) {
        //currentTrack->events.push_back(new NameEvent(calcTick(), val->s));
    }
}

void ParseContext::trackEnd()
{
    if (!checkCurrentTrack()) {
        return;
    }

    //currentTrack->events.push_back(new TrackEndEvent(calcTick()));
    currentTrack = NULL;
}

void ParseContext::timeSignature()
{
    if (!checkCurrentTrack()) {
        return;
    }

    Value *numerator = getValue(NUMERATOR);
    Value *denominator = getValue(DENOMINATOR);

    if (checkValue(numerator, NUMERATOR) && checkValue(denominator, DENOMINATOR)) {
        //currentTrack->events.push_back(new TimeSignatureEvent(calcTick(), numerator->i, denominator->i));
    }
}

void ParseContext::bankSelect()
{
    if (!checkCurrentTrack()) {
        return;
    }

    Value *channel = getValue(CHANNEL);
    Value *msb = getValue(MSB);
    Value *lsb = getValue(LSB);

    if (checkValue(channel, CHANNEL) && checkValue(msb, MSB) && checkValue(lsb, LSB)) {
        //currentTrack->events.push_back(new BankSelectEvent(calcTick(), channel->i, msb->i, lsb->i));
    }
}

void ParseContext::programChange()
{
    if (!checkCurrentTrack()) {
        return;
    }

    Value *channel = getValue(CHANNEL);
    Value *programNo = getValue(PROGRAM_CHANGE);

    if (checkValue(channel, CHANNEL) && checkValue(programNo, PROGRAM_CHANGE)) {
        //currentTrack->events.push_back(new ProgramChangeEvent(calcTick(), channel->i, programNo->i));
    }
}

void ParseContext::marker()
{
    if (!checkCurrentTrack()) {
        return;
    }

    Value *name = getValue(NAME);

    if (checkValue(name, NAME)) {
        //currentTrack->events.push_back(new MarkerEvent(calcTick(), name->s));
    }
}

void ParseContext::note()
{
    if (!checkCurrentTrack()) {
        return;
    }

    Value *channel = getValue(CHANNEL);
    Value *velocity = getValue(VELOCITY);
    Value *gatetime = getValue(GATETIME);

    if (checkValue(channel, CHANNEL) && checkValue(velocity, VELOCITY) && checkValue(gatetime, GATETIME)) {
        Value *noteNoList = getValue(NOTE_NO_LIST);
        uint32_t notes[32];
        size_t count = 0;

        char *str = noteNoList->s;
        char *token;
        while ((token = strtok(str, ","))) {
            str = NULL;
            printf("########### %s\n", token);

            if (32 <= count) {
                push_warning("note no count is more than 32. ignored.");
            }
            else {
                int noteNo = noteNo2Int(token);
                if (noteNo < 0 || 127 < noteNo) {
                    push_error("note no is out of range. C-2 to G8");
                }
                else {
                    printf("########### %d\n", noteNo);
                    notes[count++] = noteNo;
                }
            }
        }

        if (0 < count) {
            uint32_t tick = calcTick();
            for (int i = 0; i < count; ++i) {
                //currentTrack->events.push_back(new NoteEvent(tick, notes[i]));
            }
        }
    }
}

bool ParseContext::checkCurrentTrack()
{
    if (!currentTrack) {
        push_error("not inside of track.");
        return false;
    } else {
        return true;
    }
}

bool ParseContext::checkValue(Value *val, int type)
{
    if (!val) {
        push_error("%s is not set.", e2Keyword(type));
        return false;
    }

    switch (type) {
    case RESOLUTION:
        if (val->i < 24 || 960 <  val->i) {
            push_error("resolution is out of range. 24-960");
            return false;
        } 
        break;
    case PROGRAM_CHANGE:
    case VELOCITY:
    case CHANNEL:
    case MSB:
    case LSB:
        if (val->i < 0 || 127 < val->i) {
            push_error("%s is out of range. 0-127", e2Keyword(type));
            return false;
        } 
        break;
    case NUMERATOR:
        if (val->i < 1 || 32 <  val->i) {
            push_error("numerator of time signature is out of range. 1-32");
            return false;
        } 
        break;
    case DENOMINATOR:
        if (!((((((uint32_t)val->i) - 1U) & (val->i)) == 0))) {
            push_error("denominator of time signature is not power of 2.");
            return false;
        } 
        if (val->i < 2 || 32 <  val->i) {
            push_error("denominator of time signature is out of range. 2-32");
            return false;
        } 
        break;
    case GATETIME:
    case STEP:
        if (0 > val->i) {
            push_error("%s is minus.", e2Keyword(type));
            return false;
        } 
        break;
    case TEMPO:
        if (val->f < 10.0 || 300.0 < val->f) {
            push_error("tempo is out of range. 10.0-300.0");
            return false;
        } 
        break;
    }

    return true;
}

int ParseContext::noteNo2Int(char *noteNo)
{
    char octave[4] = {0};
    char *pOctave = octave;

    char *pNoteNo = noteNo;
    while (*pNoteNo) {
        *pNoteNo = toupper(*pNoteNo);
        if (*pNoteNo == '-' || *pNoteNo == '+' || isdigit(*pNoteNo)) {
            *pOctave = *pNoteNo;
            *pNoteNo = '\0';
            *(++pOctave) = '\0';
        }
        ++pNoteNo;
    }

    static std::unordered_map<std::string, int> noteMap;
    if (noteMap.empty()) {
        noteMap[std::string("C")]  = 24;
        noteMap[std::string("C#")] = 25;
        noteMap[std::string("D")]  = 26;
        noteMap[std::string("D#")] = 27;
        noteMap[std::string("E")]  = 28;
        noteMap[std::string("F")]  = 29;
        noteMap[std::string("F#")] = 30;
        noteMap[std::string("G")]  = 31;
        noteMap[std::string("G#")] = 32;
        noteMap[std::string("A")]  = 33;
        noteMap[std::string("A#")] = 34;
        noteMap[std::string("B")]  = 35;
    }

    int baseKey = noteMap[noteNo];
    if (!baseKey) {
        return -1;
    }
    else {
        currentOctaveNo = '\0' != octave[0] ? atoi(octave) : currentOctaveNo;
        return baseKey * 12 * currentOctaveNo;
    }
}
