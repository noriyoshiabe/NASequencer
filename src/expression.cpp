#include <cstdlib>
#include <cstring>
#include "tokenizer.h"
#include "expression.h"
#include "util.h"

std::string Expression::toString()
{
    return className() + ":";
}

std::string ParameterExpression::toString()
{
    std::string ret = className() + ":";
    for (std::deque<char *>::const_iterator iterator = tokenQ.begin(),
            end = tokenQ.end(); iterator != end; ++iterator) {
        ret += " ";
        ret += *iterator;
    }

    return ret;
}

ParameterExpression::ParameterExpression(std::deque<char *> *tokenQ)
{
    while (!tokenQ->empty()) {
        char *token = tokenQ->front();
        if (!strcmp(";", token) || !strcmp(Tokenizer::EOL, token)) {
            break;
        }

        this->tokenQ.push_back(token);
        tokenQ->pop_front();
    }
}
