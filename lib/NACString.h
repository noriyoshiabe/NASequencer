#pragma once

#include <alloca.h>
#include <string.h>
#include <stdbool.h>

extern char *NACStringFormat(const char *format, ...);
extern char *NACStringReplaceAll(const char *src, const char *search, const char *replacement);
extern char *NACStringJoin(const char *delimiter, const char **strings, int count);
extern char *NACStringCamel2Readable(const char *strring);

extern char *NACStringToLowerCase(char *string);
extern char *NACStringToUpperCase(char *string);
extern char *NACStringTrimWhiteSpace(char *string);

extern char *NACStringFromInteger(int i, char *buffer, int size);
extern char *NACStringFromFloat(float f, int point, char *buffer, int size);
extern char *NACStringFromChar(char c, char *buffer, int size);
extern char *NACStringFromBoolean(bool b, char *buffer, int size);
extern char *NACStringDuplicate(const char *str, char *buffer);
extern char *NACStringDuplicateNString(char *str, int length, char *buffer);
extern int NACStringSplitCount(char *str, char *delim);
extern char **NACStringSplit(char *str, char *delim, int *length, char **buffer);

#define NACStringFromInteger(i) NACStringFromInteger(i, alloca(32), 32)
#define NACStringFromFloat(f, p) NACStringFromFloat(f, p, alloca(32), 32)
#define NACStringFromChar(c) NACStringFromChar(c, alloca(2), 2)
#define NACStringFromBoolean(b) NACStringFromBoolean(b, alloca(8), 8)
#define NACStringDuplicate(s) NACStringDuplicate(s, alloca(strlen(s) + 1))
#define NACStringDuplicateNString(s, l) NACStringDuplicateNString(s, l, alloca(l + 1))
#define NACStringSplit(s, d, pl) NACStringSplit(s, d, pl, alloca(sizeof(char *) * (NACStringSplitCount(s, d) + 1)))

#define NACStringJoinWithStringArray(d, arr) NACStringJoin(d, NAArrayGetValues(arr), NAArrayCount(arr))
