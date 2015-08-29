#include "NAUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>

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
