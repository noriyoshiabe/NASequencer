#pragma once
#include <string>
#include <unordered_map>
#include "sequence.h"

union Value {
    int i;
    float f;
    char *s;
    Value(int i) { this->i = i; }
    Value(float f) { this->f = f; }
    Value(char *s) { this->s = s; }
};

class Expression;

class Context {
public:
    Expression *statement;
    Expression *modifier;

    std::unordered_map<std::string, Value *> defaultValues;
    std::unordered_map<std::string, Value *> lastValues;
    std::unordered_map<std::string, Value *> localValues;
    const char *localKey;

    uint32_t current_tick;

    Sequence *sequence;

    void setLocalKey(const char *localKey) {
        this->localKey = localKey;
    }

    void setStatement(Expression *expression) {
        statement = expression;
    }

    void setModifier(Expression *expression) {
        statement = expression;
    }

    void setLocalValue(const char *key, Value *val) {
        localValues[key] = val;
    }

    void setLocalValue(Value *val) {
        localValues[this->localKey] = val;
    }

    Value *getLocalValue(const char *key) {
        return localValues[key];
    }

    Value *getLocalValue() {
        return localValues[this->localKey];
    }

    Sequence *getSequence() {
        return sequence;
    }
};
