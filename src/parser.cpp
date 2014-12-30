#include "parser.h"
#include "util.h"

#include <cctype>

std::unordered_map<std::string, ExpressionFactory> Parser::statementTable;
std::unordered_map<std::string, ExpressionFactory> Parser::operatorTable;

Parser::Parser(const char *source)
{
    if (statementTable.empty()) {
        statementTable[Util::normalizeCase("<EOL>")] = EndOfLineExpression::create;
        statementTable[Util::normalizeCase(";")] = EndOfStatementExpression::create;
        statementTable[Util::normalizeCase("resolution")] = ResolutionExpression::create;
        statementTable[Util::normalizeCase("set")] = SetExpression::create;
        statementTable[Util::normalizeCase("unset")] = UnsetExpression::create;
        statementTable[Util::normalizeCase("default")] = DefaultModifierExpression::create;
        statementTable[Util::normalizeCase("track")] = TrackExpression::create;
        statementTable[Util::normalizeCase("time_signature")] = TimeSignatureExpression::create;
        statementTable[Util::normalizeCase("tempo")] = TempoExpression::create;
        statementTable[Util::normalizeCase("bank_select")] = BankSelectExpression::create;
        statementTable[Util::normalizeCase("program_change")] = ProgramChangeExpression::create;
        statementTable[Util::normalizeCase("marker")] = MarkerExpression::create;
        statementTable[Util::normalizeCase("note")] = NoteExpression::create;
        statementTable[Util::normalizeCase("track_end")] = TrackEndExpression::create;
        statementTable[Util::normalizeCase("include")] = IncludeExpression::create;
    }

    if (operatorTable.empty()) {
        operatorTable["="] = AssignExpression::create;
        operatorTable["+"] = PlusExpression::create;
        operatorTable["-"] = MinusExpression::create;
        operatorTable["/"] = DivisionExpression::create;
        operatorTable["."] = PointExpression::create;
        operatorTable[":"] = ColonExpression::create;
        operatorTable[","] = CommaExpression::create;
    }

    this->source = source;
}

bool Parser::isOperator(Expression *expression)
{
    return typeid(AssignExpression) == typeid(*expression)
        || typeid(PlusExpression) == typeid(*expression)
        || typeid(MinusExpression) == typeid(*expression)
        || typeid(DivisionExpression) == typeid(*expression)
        || typeid(PointExpression) == typeid(*expression)
        || typeid(ColonExpression) == typeid(*expression)
        || typeid(CommaExpression) == typeid(*expression);
}

bool Parser::canBeLeftOperand(Expression *expression)
{
    return typeid(IdentifierExpression) == typeid(*expression)
        || typeid(LiteralExpression) == typeid(*expression)
        || typeid(NumberExpression) == typeid(*expression);
}

bool Parser::isDigit(const char *str) {
    char c;
    while ((c = *(str++))) {
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

void Parser::parse(Context *context)
{
    Tokenizer *tokenizer = new Tokenizer(source);
    std::deque<Token *> *tokenQ = tokenizer->read()->createTokenQ();

    Expression *expression = SequenceExpression::create(NULL);
    ((SequenceExpression *)expression)->source = source;

    readToken(expression, tokenQ);

    Log("\n%s", expression->toString().c_str());
    expression->interpret(context);

    delete expression;
    delete tokenQ;
    delete tokenizer;
}

void Parser::readToken(Expression *parentExpression, std::deque<Token *> *tokenQ)
{
    while (!tokenQ->empty()) {
        Token *token = tokenQ->front();
        if (parentExpression->isReady(token)) {
            break;
        }

        tokenQ->pop_front();

        ExpressionFactory factory;

        if ((factory = statementTable[Util::normalizeCase(token->token)])) {
            Expression *expression = factory(token);
            readToken(expression, tokenQ);
            parentExpression->children.push_back(expression);
        }
        else if ((factory = operatorTable[token->token])) {
            Expression *expression = factory(token);

            Expression *last = parentExpression->children.empty() ? NULL : parentExpression->children.back();
            if (last && isOperator(last)) {
                Expression *_last = last;
                while (!_last->children.empty()) {
                    last = _last;
                    _last = last->children.back();
                }

                expression->children.push_back(last->children.back());
                last->children.pop_back();
                last->children.push_back(expression);
                readToken(expression, tokenQ);
            }
            else {
                if (isOperator(parentExpression)) {
                    expression->children.push_back(EmptyOperand::create(NULL));
                }
                else {
                    if (last && canBeLeftOperand(last)) {
                        parentExpression->children.pop_back();
                        expression->children.push_back(last);
                    }
                    else {
                        expression->children.push_back(EmptyOperand::create(NULL));
                    }
                }

                readToken(expression, tokenQ);
                parentExpression->children.push_back(expression);
            }
        }
        else if ('"' == token->token[0]) {
            Expression *expression = LiteralExpression::create(token);
            parentExpression->children.push_back(expression);
        }
        else if (isDigit(token->token)) {
            Expression *expression = NumberExpression::create(token);
            parentExpression->children.push_back(expression);
        }
        else {
            Expression *expression = IdentifierExpression::create(token);
            parentExpression->children.push_back(expression);
        }
    }
}
