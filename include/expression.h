#pragma once
#include <deque>
#include "context.h"

class Expression;
typedef Expression *(*ExpressionFactory)(std::deque<char *> *tokenQ);

class Expression {
protected:
    virtual std::string className() = 0;

public:
    virtual void interpret(Context *context) {}
    virtual std::string toString();

    virtual ~Expression() {}

    class Exception : public std::domain_error {
    public:
        Exception(const std::string& cause) : std::domain_error(cause) {}
    };
};

class ParameterExpression : public Expression {
protected:
    std::deque<char *> tokenQ;
public:
    std::string toString();
    ParameterExpression(std::deque<char *> *tokenQ);
};

class EndOfLineExpression : public Expression {
protected:
    std::string className() { return "EndOfLineExpression"; }
public:
    static Expression *create(std::deque<char *> *tokenQ) {
        return new EndOfLineExpression();
    }
};

class EndOfStatementExpression : public Expression {
protected:
    std::string className() { return "EndOfStatementExpression"; }
public:
    static Expression *create(std::deque<char *> *tokenQ) {
        return new EndOfStatementExpression();
    }
};

class DeltaTimeExpression : public ParameterExpression {
protected:
    std::string className() { return "DeltaTimeExpression"; }
public:
    DeltaTimeExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new DeltaTimeExpression(tokenQ);
    }
};

class SetExpression : public ParameterExpression {
protected:
    std::string className() { return "SetExpression"; }
public:
    SetExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new SetExpression(tokenQ);
    }
};

class UnsetExpression : public ParameterExpression {
protected:
    std::string className() { return "UnsetExpression"; }
public:
    UnsetExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new UnsetExpression(tokenQ);
    }
};

class TrackExpression : public ParameterExpression {
protected:
    std::string className() { return "TrackExpression"; }
public:
    TrackExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new TrackExpression(tokenQ);
    }
};

class TimeSignatureExpression : public ParameterExpression {
protected:
    std::string className() { return "TimeSignatureExpression"; }
public:
    TimeSignatureExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new TimeSignatureExpression(tokenQ);
    }
};

class TempoExpression : public ParameterExpression {
protected:
    std::string className() { return "TempoExpression"; }
public:
    TempoExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new TempoExpression(tokenQ);
    }
};

class BankSelectExpression : public ParameterExpression {
protected:
    std::string className() { return "BankSelectExpression"; }
public:
    BankSelectExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new BankSelectExpression(tokenQ);
    }
};

class ProgramChangeExpression : public ParameterExpression {
protected:
    std::string className() { return "ProgramChangeExpression"; }
public:
    ProgramChangeExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new ProgramChangeExpression(tokenQ);
    }
};

class MarkerExpression : public ParameterExpression {
protected:
    std::string className() { return "MarkerExpression"; }
public:
    MarkerExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new MarkerExpression(tokenQ);
    }
};

class NoteExpression : public ParameterExpression {
protected:
    std::string className() { return "NoteExpression"; }
public:
    NoteExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new NoteExpression(tokenQ);
    }
};

class TrackEndExpression : public ParameterExpression {
protected:
    std::string className() { return "TrackEndExpression"; }
public:
    TrackEndExpression(std::deque<char *> *tokenQ) : ParameterExpression(tokenQ) {};

    static Expression *create(std::deque<char *> *tokenQ) {
        return new TrackEndExpression(tokenQ);
    }
};
