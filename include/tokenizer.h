#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <list>

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

    State state;

    char c;
    char last;

    char buffer[Tokenizer::BUFFER_SIZE];
    int index;

    std::list<char *> tokens;

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
        return '\r' == c
            || '\n' == c;
    }

    inline bool isOperand(char c) {
        return isalpha(c) || isdigit(c)
            || c == '_'
            || c == '#';
    }

    inline bool isOperator(char c) {
        return '/' == c
            || '=' == c
            || ':' == c
            || ';' == c
            || '-' == c
            || '+' == c
            || '.' == c;
    }

    inline void pushBuffer(char c) {
        buffer[index++] = c;
        if (BUFFER_SIZE <= index) {
            static char str[128];
            sprintf(str, "Buffer size exceeded. Identifier or literal string must be less than %d, characters.", BUFFER_SIZE);
            throw Exception(str);
        }
    }

    inline void pushToken() {
        buffer[index++] = '\0';
        char *token = (char *)malloc(index);
        memcpy(token, buffer, (index));
        tokens.push_back(token);
    }

    inline void pushToken(char c) {
        char *token = (char *)malloc(2);
        token[0] = c;
        token[1] = '\0';
        tokens.push_back(token);
    }

    inline void popToken() {
        char *token = tokens.back();
        free(token);
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

    Tokenizer() {
        state = NONE;
        last = -1;
    }

    void read(FILE *fp);

    std::list<char *> *getTokens() {
        return &tokens;
    }

    class Exception {
    public:
        const char *message;

        Exception(const char *message) {
            this->message = message;
        }
    };
};
