#include <NAType.h>
#include <NAString.h>
#include <NACFHelper.h>
#include <stdio.h>

DeclareCFArrayCallBacks(__CFArrayCallBacks, NARetain, NARelease, EmptyCopyDescriptionCallBack, NAEqualTo);

int main(int argc, char **argv)
{
    NAString *str1 = (NAString *)NATypeNew(NAString, "test %d\n", 1);
    printf("-- %s\n", NAStringCString(str1));
    printf("-- %s\n", NAStringCString(NAStringAppend(str1, "test %d\n", 2)));
    printf("-- %ud\n", NAHash(str1));

    NAString *str2 = (NAString *)NATypeNew(NAString, "test %d\n", 3);
    printf("-- %s\n", NAStringCString(str2));
    printf("-- %s\n", NAStringCString(NAStringAppend(str2, "test %d\n", 4)));
    printf("-- %ud\n", NAHash(str2));

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
