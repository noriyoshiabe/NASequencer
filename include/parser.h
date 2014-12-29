#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "context.h"
#include "expression.h"

class Parser {
private:
    static std::unordered_map<std::string, ExpressionFactory> factoryTable;

    std::vector<Expression *> expressions;
    const char *source;

public:
    Parser(const char *source);
    void parse(Context *context);
};
