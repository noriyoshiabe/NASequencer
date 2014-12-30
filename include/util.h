#pragma once
#include <cctype>
#include <string>
#include <cstdio>
#include <cstdarg>

class Util {
public:
    static inline std::string normalizeCase(const char *str) {
        char buf[32];
        char *pbuf = buf;
        char c;

        while ('\0' != (c = *(str++))) {
            if ('_' != c) {
                *(pbuf++) = tolower(c);
            }
        }
        *pbuf = '\0';

        return std::string(buf);
    }

    static inline std::string format(const char *format, ...) {
        char buf[256];
        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, sizeof(buf), format, ap);
        va_end(ap);
        return std::string(buf);
    }
};

#define Log(...) printf("[%s - %d] ", __FILE__, __LINE__),printf(__VA_ARGS__),printf("\n")
#define __Trace printf("%s - %s - %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__)

