#include "parser.h"

#include <cstdio>
#include <cstring>
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

#define TO_S(sym) #sym

Sequence *Parser::sequence = NULL;

int Parser::interpret(int action, int modifier, void *arg)
{
    switch (action) {
    case STATEMENT:
        printf("%s ", TO_S(STATEMENT));
        switch(modifier) {
        case RESOLUTION: printf("%s\n", TO_S(RESOLUTION)); break;
        case SET: printf("%s\n", TO_S(SET)); break;
        case UNSET: printf("%s\n", TO_S(UNSET)); break;
        case TEMPO: printf("%s\n", TO_S(TEMPO)); break;
        case TRACK: printf("%s\n", TO_S(TRACK)); break;
        case TRACK_END: printf("%s\n", TO_S(TRACK_END)); break;
        case TIME_SIGNATURE: printf("%s\n", TO_S(TIME_SIGNATURE)); break;
        case BANK_SELECT: printf("%s\n", TO_S(BANK_SELECT)); break;
        case PROGRAM_CHANGE: printf("%s\n", TO_S(PROGRAM_CHANGE)); break;
        case MARKER: printf("%s\n", TO_S(MARKER)); break;
        case INCLUDE: printf("%s\n", TO_S(INCLUDE)); break;
        case NOTE: printf("%s\n", TO_S(NOTE)); break;
        }
        break;

    case ASSIGN:
        printf("%s ", TO_S(ASSIGN));
        switch (modifier) {
        case RESOLUTION: printf("%s=%d\n", TO_S(RESOLUTION), *((int *)arg)); break;
        case DEFAULT: printf("%s=%d\n", TO_S(DEFAULT), *((int *)arg)); break;
        case TEMPO: printf("%s=%f\n", TO_S(TEMPO), *((float *)arg)); break;
        case NUMERATOR: printf("%s=%d\n", TO_S(NUMERATOR), *((int *)arg)); break;
        case DENOMINATOR: printf("%s=%d\n", TO_S(DENOMINATOR), *((int *)arg)); break;
        case PROGRAM_CHANGE: printf("%s=%d\n", TO_S(PROGRAM_CHANGE), *((int *)arg)); break;
        case VELOCITY: printf("%s=%d\n", TO_S(VELOCITY), *((int *)arg)); break;
        case GATETIME: printf("%s=%d\n", TO_S(GATETIME), *((int *)arg)); break;
        case STEP: printf("%s=%d\n", TO_S(STEP), *((int *)arg)); break;
        case CHANNEL: printf("%s=%d\n", TO_S(CHANNEL), *((int *)arg)); break;
        case MSB: printf("%s=%d\n", TO_S(MSB), *((int *)arg)); break;
        case LSB: printf("%s=%d\n", TO_S(LSB), *((int *)arg)); break;

        case M_B_TICK: printf("%s=%s\n", TO_S(M_B_TICK), *((char **)arg)); break;
        case B_TICK: printf("%s=%s\n", TO_S(B_TICK), *((char **)arg)); break;
        case NAME: printf("%s=%s\n", TO_S(NAME), *((char **)arg)); break;
        case NOTE_NO_LIST: printf("%s=%s\n", TO_S(NOTE_NO_LIST), *((char **)arg)); break;
        }
        break;
    }

    return 0;
}

void Parser::onYyError(const char *message, int last_line, int last_column)
{
    char buf[128];
    sprintf(buf, "%s. line=%d, column=%d", message, last_line, last_column);
    throw Exception(std::string(buf));
}

Sequence *Parser::parse(const char *source)
{
    sequence = new Sequence();
    location_init();
    __interpret = Parser::interpret;
    __on_yy_error = Parser::onYyError;
    yyin = fopen(source, "r");
    yyout = fopen("/dev/null", "r");
    int ret = yyparse();
    fclose(yyin);
    return 0 == ret ? sequence : NULL;
}
