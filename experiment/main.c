#include <CoreFoundation/CoreFoundation.h>

#include <NAType.h>
#include <NAString.h>
#include <stdio.h>

const void *__CFArrayRetainCallBack(CFAllocatorRef allocator, const void *value)
{
    return NARetain(value);
}

void __CFArrayReleaseCallBack(CFAllocatorRef allocator, const void *value)
{
    NARelease(value);
}

CFStringRef __CFArrayCopyDescriptionCallBack(const void *value)
{
    return NULL;
}

Boolean __CFArrayEqualCallBack(const void *value1, const void *value2)
{
    return NAEqualTo(value1, value2);
}

const CFArrayCallBacks __CFArrayCallBacks = {
    0,
    __CFArrayRetainCallBack,
    __CFArrayReleaseCallBack,
    __CFArrayCopyDescriptionCallBack,
    __CFArrayEqualCallBack,
};

int main(int argc, char **argv)
{
    NAString *str1 = (NAString *)NATypeNew(NAString, "test %d\n", 1);
    printf("-- %s\n", NAStringCString(str1));
    printf("-- %s\n", NAStringCString(NAStringAppend(str1, "test %d\n", 2)));

    NAString *str2 = (NAString *)NATypeNew(NAString, "test %d\n", 3);
    printf("-- %s\n", NAStringCString(str2));
    printf("-- %s\n", NAStringCString(NAStringAppend(str2, "test %d\n", 4)));

    CFMutableArrayRef array = CFArrayCreateMutable(NULL, 0, &__CFArrayCallBacks);
    CFArrayAppendValue(array, str1);
    CFArrayAppendValue(array, str2);

    NARelease(str1);
    NARelease(str2);

    printf("--- NAReleased\n");

    CFRelease(array);

    printf("--- CAReleased\n");

    return 0;
}
