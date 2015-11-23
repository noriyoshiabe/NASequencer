#include "NAUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>

char *NAUtilGetRealPath(const char *filepath)
{
    char buf[PATH_MAX];
    char *_filepath = realpath(filepath, buf);
    if (!_filepath) {
        return NULL;
    }

    char *ret = malloc(strlen(_filepath) + 1);
    strcpy(ret, _filepath);
    return ret;
}

char *NAUtilBuildPathWithDirectory(const char *directory, const char *filename)
{
    char buf[PATH_MAX];
    snprintf(buf, PATH_MAX, "%s/%s", directory, filename);
    char *ret = malloc(strlen(buf) + 1);
    strcpy(ret, buf);
    return ret;
}

const char *NAUtilGetFileExtenssion(const char *filepath)
{
    const char *dot = strrchr(filepath, '.');
    
    if (!dot || dot == filepath) {
        return "";
    }

    return dot + 1;
}

const char *NAUtilGetLastPathComponent(const char *filepath)
{
    const char *slash = strrchr(filepath, '/');
    return slash ? slash + 1 : filepath;
}

char *NAUtilToLowerCase(char *string)
{
    char *c = string;
    while (*c) {
        *c = tolower(*c);
        ++c;
    }
    return string;
}

char *NAUtilTrimWhiteSpace(char *string)
{
    int len = strlen(string);
    char *pc = string;
    for (pc = string + len - 1; string <= pc && isspace(*pc); --pc) {
        *pc = '\0';
    }

    pc = string;
    while (isspace(*pc)) ++pc;

    memmove(string, pc, len + 1);
    return string;
}

bool NAUtilIsDirectory(char *path)
{
    struct stat s;
    return 0 == stat(path, &s) && s.st_mode & S_IFDIR;
}

char *NAUtilFormatString(const char *format, ...)
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
