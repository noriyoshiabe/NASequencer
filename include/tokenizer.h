#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <list>
#include <deque>
#include <string>
#include <sstream>

struct Token {
    char *token;
    const char *file;
    int line;

    Token(char *token, const char *file, int line) {
        this->token = token;
        this->file = file;
        this->line = line;
    }

    ~Token() {
        free(token);
    }

    std::string toString() const {
        std::ostringstream stringStream;
        stringStream << file << " - " << line << " - " << token;
        return stringStream.str();
    }
};

class Tokenizer {
private:

    enum State {
        NONE,
        OPERAND,
        SINGLE_QUOTE_LITERAL,
        DOUBLE_QUOTE_LITERAL,
        LINE_COMMENT,
        BLOCK_COMMENT,
    };

    static const int BUFFER_SIZE = 128;

    const char *source;

    State state;

    char c;
    char last;

    char buffer[Tokenizer::BUFFER_SIZE];
    int index;

    std::list<Token *> tokens;

    int line = 1;

    inline bool isLineCommentStart(char c) {
        return '/' == c && last == '/';
    }

    inline bool isBlockCommentStart(char c) {
        return '*' == c && last == '/';
    }

    inline bool isLineCommentEnd(char c) {
        return '\r' == c || '\n' == c;
    }

    inline bool isBlockCommentEnd(char c) {
        return '/' == c && '*' == last;
    }

    inline bool isLineBreak(char c) {
        return '\r' == c || ('\n' == c && '\r' != last);
    }

    inline bool isOperand(char c) {
        return isalpha(c) || isdigit(c)
            || c == '_'
            || c == '#';
    }

    inline bool isOperator(char c) {
        return '=' == c
            || '+' == c
            || '-' == c
            || '/' == c
            || '.' == c
            || ':' == c
            || ',' == c
            || ';' == c;
    }

    inline void pushBuffer(char c) {
        buffer[index++] = c;
        if (BUFFER_SIZE <= index) {
            throw Exception("Identifier or literal string must be less than " + std::to_string(BUFFER_SIZE) + " characters.");
        }
    }

    inline void pushToken(const char *str) {
        pushToken(str, strlen(str));
    }

    inline void pushToken(const char *str, int length) {
        char *token = (char *)malloc(length);
        memcpy(token, str, length);
        tokens.push_back(new Token(token, source, line));
    }

    inline void pushToken() {
        buffer[index++] = '\0';
        pushToken(buffer, index);
    }

    inline void pushToken(char c) {
        char token[] = {c, '\0'};
        pushToken(token, sizeof(token));
    }

    inline void pushLiteral() {
        char *token = (char *)malloc(index + 2);

        memcpy(token + 1, buffer, index);
        token[0] = '"';
        token[index + 1] = '\0';

        tokens.push_back(new Token(token, source, line));
    }

    inline void popToken() {
        Token *token = tokens.back();
        delete token;
        tokens.pop_back();
    }

    inline void stateChange(State state) {
        this->state = state;
        switch (state) {
        case NONE:
        case SINGLE_QUOTE_LITERAL:
        case DOUBLE_QUOTE_LITERAL:
            index = 0;
            break;
        default:
            break;
        }
    }

public:
    static const char *EOL;

    Tokenizer(const char *source) {
        this->source = source;
        state = NONE;
        last = -1;
    }

    ~Tokenizer() {
        clearTokens();
    }

    Tokenizer *read();

    std::deque<Token *> *createTokenQ() {
        return new std::deque<Token *>(tokens.begin(), tokens.end());
    }

    Tokenizer *clearTokens() {
        for (Token *token : tokens) {
            delete token;
        }
        return this;
    }

    class Exception : public std::domain_error {
    public:
        Exception(const std::string& cause) : std::domain_error(cause) {}
    };
};
