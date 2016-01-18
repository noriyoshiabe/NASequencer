#include "NACString.h"
#include "NAStringBuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#undef NACStringFromInteger
#undef NACStringFromFloat
#undef NACStringFromChar
#undef NACStringFromBoolean
#undef NACStringDuplicate
#undef NACStringDuplicateNString
#undef NACStringSplit

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

char *NACStringReplaceAll(const char *src, const char *search, const char *replacement)
{
    NAStringBuffer *buffer = NAStringBufferCreate(128);

    int searchLength = strlen(search);

    const char *pc = src;
    while (*pc) {
        const char *s = strstr(pc, search);
        if (!s) {
            NAStringBufferAppendString(buffer, pc);
            break;
        }

        int from = s - pc;
        NAStringBufferAppendNString(buffer, pc, from);
        NAStringBufferAppendString(buffer, replacement);
        pc += from + searchLength;
    }

    char *ret = NAStringBufferRetriveCString(buffer);
    NAStringBufferDestroy(buffer);
    return ret;
}

char *NACStringJoin(const char *delimiter, const char **strings, int count)
{
    NAStringBuffer *buffer = NAStringBufferCreate(128);

    for (int i = 0; i < count; ++i) {
        if (0 < i) {
            NAStringBufferAppendString(buffer, delimiter);
        }
        NAStringBufferAppendString(buffer, strings[i]);
    }

    char *ret = NAStringBufferRetriveCString(buffer);
    NAStringBufferDestroy(buffer);
    return ret;
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

char *NACStringDuplicate(const char *str, char *buffer)
{
    strcpy(buffer, str);
    return buffer;
}

char *NACStringDuplicateNString(char *str, int length, char *buffer)
{
    strncpy(buffer, str, length);
    buffer[length] = '\0';
    return buffer;
}

int NACStringSplitCount(char *str, char *delim)
{
    int ret = 0;
    char *pc = str;
    bool delimiter = true;
    
    while (*pc) {
        if (strchr(delim, *pc)) {
            delimiter = true;
        }
        else {
            if (delimiter) {
                ++ret;
            }
            delimiter = false;
        }
        ++pc;
    }

    return ret;
}

char **NACStringSplit(char *str, char *delim, int *length, char **buffer)
{
    if (length) {
        *length = 0;
    }

    char **pbuf = buffer;
    char *saveptr, *token, *s = str;
    while ((token = strtok_r(s, delim, &saveptr))) {
        *(pbuf++) = token;
        if (length) {
            ++(*length);
        }
        s = NULL;
    }

    *pbuf = NULL;

    return buffer;
}
