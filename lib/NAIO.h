#pragma once

#include <stdio.h>
#include <stdbool.h>

extern char *NAIOGetRealPath(const char *filepath);
extern char *NAIOBuildPathWithDirectory(const char *directory, const char *filename);
extern const char *NAIOGetFileExtenssion(const char *filepath);
extern const char *NAIOGetLastPathComponent(const char *filepath);
extern bool NAIOIsDirectory(char *path);
extern FILE *NAIOCreateMemoryStream(int allocationUnit);
