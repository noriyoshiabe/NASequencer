#include <cstdlib>
#include <cstring>
#include "tokenizer.h"
#include "expression.h"
#include "util.h"

bool RootExpression::isReady(const Token *token)
{
    return false;
}

bool StatementExpression::isReady(const Token *token)
{
    return !strcmp(";", token->token) || !strcmp(Tokenizer::EOL, token->token);
}

bool OperatorExpression::isReady(const Token *token)
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

    const char *_token = NULL != token ? token->token : "";
    std::string ret = indent + className() + ": " + _token + "\n";
    for (Expression *expression : children) {
        ret += expression->toString(depth + 1);
    }

    return ret;
}

void SequenceExpression::interpret(Context *context)
{
    context->sequence = new Sequence();
    interpretChild(context);
}

void EndOfLineExpression::interpret(Context *context)
{
}

void EndOfStatementExpression::interpret(Context *context)
{
}

void ResolutionExpression::interpret(Context *context)
{
    context->statement = this;
    context->setLocalKey(token->token);

    interpretChild(context);

    Value *val = context->getLocalValue();
    if (!val) {
        throw Exception(token->toString() + ": value is missing.");
    }

    if (0 < val->i && val->i <= 960) {
        context->getSequence()->resolution = val->i;
    }
    else {
        throw Exception(token->toString() + ": invalid range. val=" + std::to_string(val->i));
    }
}

void SetExpression::interpret(Context *context)
{
}
void UnsetExpression::interpret(Context *context) { }
void DefaultModifierExpression::interpret(Context *context) { }
void TrackExpression::interpret(Context *context) { }
void TimeSignatureExpression::interpret(Context *context) { }
void TempoExpression::interpret(Context *context) { }
void BankSelectExpression::interpret(Context *context) { }
void ProgramChangeExpression::interpret(Context *context) { }
void MarkerExpression::interpret(Context *context) { }
void NoteExpression::interpret(Context *context) { }
void TrackEndExpression::interpret(Context *context) { }
void IncludeExpression::interpret(Context *context) { }
void AssignExpression::interpret(Context *context) { }
void PlusExpression::interpret(Context *context) { }
void MinusExpression::interpret(Context *context) { }
void DivisionExpression::interpret(Context *context) { }
void PointExpression::interpret(Context *context) { }
void ColonExpression::interpret(Context *context) { }
void CommaExpression::interpret(Context *context) { }
void LiteralExpression::interpret(Context *context) { }

void NumberExpression::interpret(Context *context)
{
    context->setLocalValue(new Value(atoi(token->token)));
}

void IdentifierExpression::interpret(Context *context) { }
void EmptyOperand::interpret(Context *context) { }
