#pragma once

#include "sequence.h"

class Parser {
private:
    static Sequence *sequence;
public:
    static int interpret(int action, int modifier, void *arg);
    static void onYyError(const char *s, int last_line, int last_column);

    static Sequence *parse(const char *source);

    class Exception : public std::domain_error {
    public:
        Exception(const std::string& cause) : std::domain_error(cause) {}
    };
};
