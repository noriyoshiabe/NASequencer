#include "natype.h"
#include "hoge.h"
#include "hage.h"

int main(int argc, char **argv)
{
    Hoge *hoge = HogeCrate();
    Hoge *hage = HageCrate();

    CFMutableArrayRef array = CFArrayCreateMutable(NULL, 0, &__CFArrayCallBacks);
    CFArrayAppendValue(array, hoge);
    CFArrayAppendValue(array, hage);

    HogeDoHoge(hoge);
    HogeDoHoge(hage);

    NARelease(hoge);
    NARelease(hage);

    printf("--- NAReleased\n");

    CFRelease(array);

    printf("--- CAReleased\n");

    return 0;
}
