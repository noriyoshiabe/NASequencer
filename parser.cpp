#include "parser.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <string>
#include "namidi.tab.h"

extern "C" {

extern void location_init();
extern int yyparse();
extern FILE *yyin;
extern FILE *yyout;
extern int (*__interpret)(int action, int modifier, void *arg);
extern void (*__on_yy_error)(const char *message, int last_line, int last_column);

}

ParseContext *Parser::_context = NULL;

int Parser::interpret(int action, int modifier, void *arg)
{
    return _context->interpret(action, modifier, arg);
}

void Parser::onYyError(const char *message, int last_line, int last_column)
{
    _context->push_error("%s. line=%d, column=%d", message, last_line, last_column);
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
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    errors.push_back(new std::string(buf));
}

void ParseContext::push_warning(const char *format, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
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
            break;
        case TRACK_END:
            break;
        case TIME_SIGNATURE:
            break;
        case BANK_SELECT:
            break;
        case PROGRAM_CHANGE:
            break;
        case MARKER:
            break;
        case INCLUDE:
            break;
        case NOTE:
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
    if ((val = localValues[e])) {
        return val;
    }
    if ((val = defaultValues[e])) {
        return val;
    }
    if ((val = lastValues[e])) {
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

    if (localValues[modifier]) {
        delete localValues[modifier];
    }
    localValues[modifier] = val;
#undef _C
}

void ParseContext::updateLastValues(int statement)
{
    std::vector<int> uselessValues;

    for (auto it = localValues.begin(); it != localValues.end();) {
        Value *val = lastValues[it->first];
        if (val) {
            lastValues.erase(it->first);
            delete val;
        }

        if (isUselessValue(statement, it->first)) {
            uselessValues.push_back(it->first);
        }

        lastValues[it->first] = it->second;
        it = localValues.erase(it);
    }

    for (int assign : uselessValues) {
        push_warning("[%s] is useless for [%s] statement.", e2Key(assign), e2Key(statement));
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
    Value *val = localValues[RESOLUTION];
    if (val) {
        sequence->resolution = val->i;
    } else {
        push_error("reslution value not found.");
    }
}

void ParseContext::set()
{
    Value *val = localValues[DEFAULT];
    if (val) {
        localValues.erase(DEFAULT);
        delete val;

        for (std::pair<int, Value *> entry : localValues) {
            val = defaultValues[entry.first];
            if (val) {
                delete val;
            }
            defaultValues[entry.first] = entry.second;
        }

        localValues.clear();
    }
}

void ParseContext::unset()
{
    Value *val = localValues[DEFAULT];
    if (val) {
        localValues.erase(DEFAULT);
        delete val;

        for (std::pair<int, Value *> entry : localValues) {
            val = defaultValues[entry.first];
            if (val) {
                defaultValues.erase(entry.first);
                delete val;
            }
            delete entry.second;
        }
    }
    else {
        for (std::pair<int, Value *> entry : localValues) {
            val = lastValues[entry.first];
            if (val) {
                lastValues.erase(entry.first);
                delete val;
            }
            delete entry.second;
        }
    }

    localValues.clear();
}

void ParseContext::tempo()
{
    Value *val = localValues[TEMPO];
    if (!val) {
        push_error("tempo value is not set.");
    } else {
        float tempo = val->f;
        uint32_t tick = calcTick();
        //context->currentTrack->events.push_back(MidiEventFactory::createTempoEvent(tick, tempo));
    }
}
