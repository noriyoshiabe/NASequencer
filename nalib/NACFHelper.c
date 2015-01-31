#include <NACFHelper.h>
#include <NAType.h>

static const void *__CFArrayRetainCallBack(CFAllocatorRef allocator, const void *value)
{
    return NARetain((void *)value);
}

static void __CFArrayReleaseCallBack(CFAllocatorRef allocator, const void *value)
{
    NARelease((void *)value);
}

static CFStringRef __CFArrayCopyDescriptionCallBack(const void *value)
{
    return NADescription(value);
}

static Boolean __CFArrayEqualCallBack(const void *value1, const void *value2)
{
    return NAEqual(value1, value2);
}

static const CFArrayCallBacks __NACFArrayCallBacks = {
    0,
    __CFArrayRetainCallBack,
    __CFArrayReleaseCallBack,
    __CFArrayCopyDescriptionCallBack,
    __CFArrayEqualCallBack
};

const CFArrayCallBacks *NACFArrayCallBacks = &__NACFArrayCallBacks;


static const void *__CFDictionaryRetainCallBack(CFAllocatorRef allocator, const void *value)
{
    return NARetain((void *)value);
}

static void __CFDictionaryReleaseCallBack(CFAllocatorRef allocator, const void *value)
{
    NARelease((void *)value);
}

static CFStringRef __CFDictionaryCopyDescriptionCallBack(const void *value)
{
    return NADescription(value);
}

static Boolean __CFDictionaryEqualCallBack(const void *value1, const void *value2)
{
    return NAEqual(value1, value2);
}

static CFHashCode __CFDictionaryHashCallBack(const void *value)
{
    return NAHash(value);
}

static const CFDictionaryKeyCallBacks __NACFDictionaryKeyCallBacks = {
    0,
    __CFDictionaryRetainCallBack,
    __CFDictionaryReleaseCallBack,
    __CFDictionaryCopyDescriptionCallBack,
    __CFDictionaryEqualCallBack,
    __CFDictionaryHashCallBack,
};

const CFDictionaryKeyCallBacks *NACFDictionaryKeyCallBacks = &__NACFDictionaryKeyCallBacks;

static const CFDictionaryValueCallBacks __NACFDictionaryValueCallBacks = {
    0,
    __CFDictionaryRetainCallBack,
    __CFDictionaryReleaseCallBack,
    __CFDictionaryCopyDescriptionCallBack,
    __CFDictionaryEqualCallBack
};

const CFDictionaryValueCallBacks *NACFDictionaryValueCallBacks = &__NACFDictionaryValueCallBacks;
