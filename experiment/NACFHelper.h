#pragma once

#include <CoreFoundation/CoreFoundation.h>

static inline const void *EmptyRetainCallBack(CFAllocatorRef allocator, const void *value)
{
    return value;
}

static inline void EmptyReleaseCallBack(CFAllocatorRef allocator, const void *value)
{
}

static inline CFStringRef EmptyCopyDescriptionCallBack(const void *value)
{
    return NULL;
}

static inline Boolean EmptyEqualCallBack(const void *value1, const void *value2)
{
    return false;
}

#define DeclareCFArrayCallBacks(name, RetainCallBack, ReleaseCallBack, CopyDescriptionCallBack, EqualCallBack) \
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
        return CopyDescriptionCallBack == NULL ? NULL : NULL; \
    } \
     \
    Boolean __##name##__CFArrayEqualCallBack(const void *value1, const void *value2) \
    { \
        return NAEqualTo(value1, value2); \
    } \
     \
    const CFArrayCallBacks name = { \
        0, \
        __##name##__CFArrayRetainCallBack, \
        __##name##__CFArrayReleaseCallBack, \
        __##name##__CFArrayCopyDescriptionCallBack, \
        __##name##__CFArrayEqualCallBack, \
    }
