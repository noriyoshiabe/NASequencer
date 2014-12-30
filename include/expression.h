#pragma once
#include <deque>
#include "context.h"

class Expression;
typedef Expression *(*ExpressionFactory)(const char *token);

class Expression {
private:
    const char *originalToken;

public:
    std::deque<Expression *> children;
    std::string toString();
    std::string toString(int depth);

    virtual std::string className() = 0;
    virtual bool isReady(const char *token) {
        return true;
    }

    virtual void interpret(Context *context) {}

    Expression(const char *token) {
        originalToken = token;
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
    RootExpression(const char *token) : Expression(token) {};
    bool isReady(const char *token);
};

class StatementExpression : public Expression {
public:
    StatementExpression(const char *token) : Expression(token) {};
    bool isReady(const char *token);
};

class OperatorExpression : public Expression {
public:
    OperatorExpression(const char *token) : Expression(token) {};
    bool isReady(const char *token);
};

#define DeclareExpresssion(clazz, parent) \
    class clazz : public parent { \
    public: \
        clazz(const char *token) : parent(token) {}; \
        std::string className() { return #clazz; } \
        static Expression *create(const char *token) { \
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
