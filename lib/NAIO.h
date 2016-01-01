#pragma once

#include <stdio.h>
#include <stdbool.h>

#define NATrace printf("-- %s - %d - %s\n", __FILE__, __LINE__, __FUNCTION__)
#define NATraceLog(fmt, __VA_ARGS__) printf("-- %s - %d - %s: ", __FILE__, __LINE__, __FUNCTION__), printf(fmt, __VA_ARGS__), printf("\n")

extern char *NAIOGetRealPath(const char *filepath);
extern char *NAIOBuildPathWithDirectory(const char *directory, const char *filename);
extern const char *NAIOGetFileExtenssion(const char *filepath);
extern const char *NAIOGetLastPathComponent(const char *filepath);
extern bool NAIOIsDirectory(char *path);
extern FILE *NAIOCreateMemoryStream(int allocationUnit);
