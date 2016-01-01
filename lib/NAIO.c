#include "NAIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <sys/stat.h>

char *NAIOGetRealPath(const char *filepath)
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

char *NAIOBuildPathWithDirectory(const char *directory, const char *filename)
{
    char buf[PATH_MAX];
    snprintf(buf, PATH_MAX, "%s/%s", directory, filename);
    char *ret = malloc(strlen(buf) + 1);
    strcpy(ret, buf);
    return ret;
}

const char *NAIOGetFileExtenssion(const char *filepath)
{
    const char *dot = strrchr(filepath, '.');
    
    if (!dot || dot == filepath) {
        return "";
    }

    return dot + 1;
}

const char *NAIOGetLastPathComponent(const char *filepath)
{
    const char *slash = strrchr(filepath, '/');
    return slash ? slash + 1 : filepath;
}

bool NAIOIsDirectory(char *path)
{
    struct stat s;
    return 0 == stat(path, &s) && s.st_mode & S_IFDIR;
}
