#pragma once
#include <deque>
#include "context.h"
#include "tokenizer.h"

class Expression;
typedef Expression *(*ExpressionFactory)(const Token *token);

class Expression {
protected:
    const Token *token;
    void interpretChild(Context *context) {
        for (Expression *expression : children) {
            expression->interpret(context);
        }
    }

public:
    std::deque<Expression *> children;
    std::string toString();
    std::string toString(int depth);

    virtual std::string className() = 0;
    virtual bool isReady(const Token *token) {
        return true;
    }

    virtual void interpret(Context *context) {}

    Expression(const Token *token) {
        this->token = token;
    }

    virtual ~Expression() {
        for (Expression *expression : children) {
            delete expression;
        }
    }

    class Exception : public std::domain_error {
    public:
        Exception(const std::string& cause) : std::domain_error(cause) {}
    };
};

class RootExpression : public Expression {
public:
    RootExpression(const Token *token) : Expression(token) {};
    bool isReady(const Token *token);

    const char *source;
};

class StatementExpression : public Expression {
public:
    StatementExpression(const Token *token) : Expression(token) {};
    bool isReady(const Token *token);
};

class OperatorExpression : public Expression {
public:
    OperatorExpression(const Token *token) : Expression(token) {};
    bool isReady(const Token *token);
};

#define DeclareExpresssion(clazz, parent) \
    class clazz : public parent { \
    public: \
        clazz(const Token *token) : parent(token) {}; \
        std::string className() { return #clazz; } \
        void interpret(Context *context); \
        static Expression *create(const Token *token) { \
            return new clazz(token); \
        } \
    };

DeclareExpresssion(SequenceExpression, RootExpression);

DeclareExpresssion(EndOfLineExpression, Expression);
DeclareExpresssion(EndOfStatementExpression, Expression);

DeclareExpresssion(ResolutionExpression, StatementExpression);
DeclareExpresssion(SetExpression, StatementExpression);
DeclareExpresssion(UnsetExpression, StatementExpression);
DeclareExpresssion(DefaultModifierExpression, StatementExpression);
DeclareExpresssion(TrackExpression, StatementExpression);
DeclareExpresssion(TimeSignatureExpression, StatementExpression);
DeclareExpresssion(TempoExpression, StatementExpression);
DeclareExpresssion(BankSelectExpression, StatementExpression);
DeclareExpresssion(ProgramChangeExpression, StatementExpression);
DeclareExpresssion(MarkerExpression, StatementExpression);
DeclareExpresssion(NoteExpression, StatementExpression);
DeclareExpresssion(TrackEndExpression, StatementExpression);
DeclareExpresssion(IncludeExpression, StatementExpression);

DeclareExpresssion(AssignExpression, OperatorExpression);
DeclareExpresssion(PlusExpression, OperatorExpression);
DeclareExpresssion(MinusExpression, OperatorExpression);
DeclareExpresssion(DivisionExpression, OperatorExpression);
DeclareExpresssion(PointExpression, OperatorExpression);
DeclareExpresssion(ColonExpression, OperatorExpression);
DeclareExpresssion(CommaExpression, OperatorExpression);

DeclareExpresssion(LiteralExpression, Expression);
DeclareExpresssion(NumberExpression, Expression);
DeclareExpresssion(IdentifierExpression, Expression);
DeclareExpresssion(EmptyOperand, Expression);
