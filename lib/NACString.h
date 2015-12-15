#pragma once

#include <alloca.h>
#include <string.h>

extern char *NACStringFormat(const char *format, ...);
extern char *NACStringToLowerCase(char *string);
extern char *NACStringTrimWhiteSpace(char *string);

extern char *NACStringFromInteger(int i, char *buffer, int size);
extern char *NACStringFromFloat(float f, int point, char *buffer, int size);
extern char *NACStringFromChar(char c, char *buffer, int size);
extern char *NACStringDuplicate(char *str, char *buffer);

#define NACStringFromInteger(i) NACStringFromInteger(i, alloca(32), 32)
#define NACStringFromFloat(f, p) NACStringFromFloat(f, p, alloca(32), 32)
#define NACStringFromChar(c) NACStringFromChar(c, alloca(2), 2)
#define NACStringDuplicate(s) NACStringDuplicate(s, alloca(strlen(s) + 1))
