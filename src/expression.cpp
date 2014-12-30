#include <cstdlib>
#include <cstring>
#include "tokenizer.h"
#include "expression.h"
#include "util.h"

bool RootExpression::isReady(const char *token)
{
    return false;
}

bool StatementExpression::isReady(const char *token)
{
    return !strcmp(";", token) || !strcmp(Tokenizer::EOL, token);
}

bool OperatorExpression::isReady(const char *token)
{
    return 2 <= children.size();
}

std::string Expression::toString()
{
    return toString(0);
}

std::string Expression::toString(int depth)
{
    std::string indent = "";
    for (int i = 0; i < depth; ++i) {
        indent += "    ";
    }

    std::string ret = indent + className() + ": " + originalToken + "\n";
    for (Expression *expression : children) {
        ret += expression->toString(depth + 1);
    }

    return ret;
}
