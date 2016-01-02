#include "NACString.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#undef NACStringFromInteger
#undef NACStringFromFloat
#undef NACStringFromChar
#undef NACStringFromBoolean
#undef NACStringDuplicate

char *NACStringFormat(const char *format, ...)
{
    va_list argList;

    va_start(argList, format);
    int length = vsnprintf(NULL, 0, format, argList);
    va_end(argList);

    char *buffer = malloc(length + 1);

    va_start(argList, format);
    vsnprintf(buffer, length + 1, format, argList);
    va_end(argList);

    return buffer;
}

char *NACStringToLowerCase(char *string)
{
    char *c = string;
    while (*c) {
        *c = tolower(*c);
        ++c;
    }
    return string;
}

char *NACStringToUpperCase(char *string)
{
    char *c = string;
    while (*c) {
        *c = tolower(*c);
        ++c;
    }
    return string;
}

char *NACStringTrimWhiteSpace(char *string)
{
    int len = strlen(string);
    char *pc = string;
    for (pc = string + len - 1; string <= pc && isspace(*pc); --pc) {
        *pc = '\0';
    }

    pc = string;
    while (isspace(*pc)) ++pc;

    memmove(string, pc, len - (pc - string) + 1);
    return string;
}

char *NACStringFromInteger(int i, char *buffer, int size)
{
    snprintf(buffer, size, "%d", i);
    return buffer;
}

char *NACStringFromFloat(float f, int point, char *buffer, int size)
{
    snprintf(buffer, size, "%.*f", point, f);
    return buffer;
}

char *NACStringFromChar(char c, char *buffer, int size)
{
    snprintf(buffer, size, "%c", c);
    return buffer;
}

char *NACStringFromBoolean(bool b, char *buffer, int size)
{
    snprintf(buffer, size, "%s", b ? "true" : "false");
    return buffer;
}

char *NACStringDuplicate(char *str, char *buffer)
{
    strcpy(buffer, str);
    return buffer;
}
