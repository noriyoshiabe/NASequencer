#pragma once

#include <CoreFoundation/CoreFoundation.h>

#define DeclareCFArrayCallBacks(name, CopyDescriptionCallBack) \
const void *__##name##__CFArrayRetainCallBack(CFAllocatorRef allocator, const void *value) \
    { \
        return NARetain(value); \
    } \
     \
    void __##name##__CFArrayReleaseCallBack(CFAllocatorRef allocator, const void *value) \
    { \
        NARelease(value); \
    } \
     \
    CFStringRef __##name##__CFArrayCopyDescriptionCallBack(const void *value) \
    { \
        return CopyDescriptionCallBack != NULL ? CopyDescriptionCallBack : NULL; \
    } \
     \
    Boolean __##name##__CFArrayEqualCallBack(const void *value1, const void *value2) \
    { \
        return NAEqual(value1, value2); \
    } \
     \
    const CFArrayCallBacks name = { \
        0, \
        __##name##__CFArrayRetainCallBack, \
        __##name##__CFArrayReleaseCallBack, \
        __##name##__CFArrayCopyDescriptionCallBack, \
        __##name##__CFArrayEqualCallBack, \
    }
