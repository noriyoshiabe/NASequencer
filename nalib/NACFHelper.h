#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <alloca.h>

extern const CFArrayCallBacks *NACFArrayCallBacks;
extern const CFDictionaryKeyCallBacks *NACFDictionaryKeyCallBacks;
extern const CFDictionaryValueCallBacks *NACFDictionaryValueCallBacks;

extern const CFComparatorFunction NACFComparatorFunction;
extern const CFComparatorFunction NACFComparatorFunctionDesc;

#define NACFString2CString(cfString) __NACFString2CString(cfString, alloca(CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfString), kCFStringEncodingUTF8) + 1))
extern const char *__NACFString2CString(CFStringRef cfString, char *allocaBuffer);
