#pragma once

#include "sequence.h"

#include <string>
#include <vector>
#include <unordered_map>

class ParseContext;

class Parser {
private:
    static ParseContext *_context;
public:
    static int interpret(int action, int modifier, void *arg);
    static void onYyError(const char *s, int last_line, int last_column);

    static ParseContext *parse(const char *source);

    class Exception : public std::domain_error {
    public:
        Exception(const std::string& cause) : std::domain_error(cause) {}
    };
};

struct Value {
    int i;
    float f;
    char *s;
};

class ParseContext {
private:
    Sequence *sequence;

    std::unordered_map<int, Value *> localValues;
    std::unordered_map<int, Value *> lastValues;
    std::unordered_map<int, Value *> defaultValues;

    uint32_t currentTick = 0;
    Track *currentTrack = NULL;

    const char* e2Key(int e);
    const char* e2Keyword(int e);
    Value *getValue(int e);
    uint32_t calcTick();
    void assign(int modifier, void *arg);
    void updateLastValues(int statement);
    bool isUselessValue(int statement, int assign);
    int parseMBTick(char *str, bool includeMeasure);
    void resolution();
    void set();
    void unset();
    void tempo();

public:
    int interpret(int action, int modifier, void *arg);

    void push_error(const char *format, ...);
    void push_warning(const char *format, ...);

    std::vector<std::string *> errors;
    std::vector<std::string *> warnings;

    ParseContext() {
        sequence = new Sequence();
    }

    ~ParseContext() {
        if (sequence) {
            delete sequence;
        }

        for (std::string *error : errors) {
            delete error;
        }

        for (std::string *warning : warnings) {
            delete warning;
        }
    }

    Sequence *getSequence() {
        Sequence *ret = sequence;
        sequence = NULL;
        return ret;
    }
};
