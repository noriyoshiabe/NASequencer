#pragma once

#include <string>
#include <unordered_map>
#include "context.h"
#include "expression.h"

class Parser {
private:
    static std::unordered_map<std::string, ExpressionFactory> statementTable;
    static std::unordered_map<std::string, ExpressionFactory> operatorTable;

    const char *source;

    bool isDigit(const char *str);
    bool isOperator(Expression *expression);
    bool canBeLeftOperand(Expression *expression);

public:
    Parser(const char *source);
    void parse(Context *context);
    void readToken(Expression *parentExpression, std::deque<char *> *tokenQ);
};
