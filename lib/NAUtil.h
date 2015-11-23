#pragma once

#include <stdbool.h>
#include <stdarg.h>

#define NATrace printf("-- %s - %d - %s\n", __FILE__, __LINE__, __FUNCTION__)
#define NATraceLog(fmt, __VA_ARGS__) printf("-- %s - %d - %s: ", __FILE__, __LINE__, __FUNCTION__), printf(fmt, __VA_ARGS__), printf("\n")

extern char *NAUtilGetRealPath(const char *filepath);
extern char *NAUtilBuildPathWithDirectory(const char *directory, const char *filename);
extern const char *NAUtilGetFileExtenssion(const char *filepath);
extern const char *NAUtilGetLastPathComponent(const char *filepath);
extern char *NAUtilToLowerCase(char *string);
extern char *NAUtilTrimWhiteSpace(char *string);
extern bool NAUtilIsDirectory(char *path);
extern char *NAUtilFormatString(const char *format, ...);
