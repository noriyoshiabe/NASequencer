#include <NAType.h>
#include <NACFHelper.h>
#include <stdio.h>

#define __Trace printf("---- %s:%s - %d\n", __FILE__, __func__, __LINE__)

extern NAClass HogeClass;
char HogeID[] = "Hoge";

typedef struct _Hoge {
    NAType _;
    int hogeValue;
} Hoge;

typedef struct __HogeVtbl {
    void (*sayHoge)(void *self);
} HogeVtbl;

static void *__HogeInit(void *_self, ...)
{
    __Trace;

    Hoge *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->hogeValue = va_arg(ap, int);
    va_end(ap);

    printf("self->hogeValue=%d\n", self->hogeValue);

    return self;
}

static void __HogeDestroy(void *self)
{
    __Trace;
}

static uint32_t __HogeHash(const void *self)
{
    __Trace;
    return (uint32_t)self >> 2;
}

static bool __HogeEqualTo(const void *self, const void *to)
{
    __Trace;
    return ((Hoge *)self)->hogeValue == ((Hoge *)to)->hogeValue;
}

static int __HogeCompare(const void *self, const void *to)
{
    __Trace;
    return ((Hoge *)self)->hogeValue - ((Hoge *)to)->hogeValue;
}

static void *__HogeCopy(const void *self)
{
    __Trace;
    return NATypeNew(Hoge, ((Hoge *)self)->hogeValue);
}

static void *__HogeDescription(const void *self)
{
    __Trace;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<Hoge: hogeValue=%d>"), ((const Hoge *)self)->hogeValue);
}

static void __HogeSayHoge(void *self)
{
    __Trace;
    printf("Hoge!! hogeValue=%d\n", ((Hoge *)self)->hogeValue);
}

void HogeSayHoge(void *self)
{
     NAVtbl(self, Hoge)->sayHoge(self);
}

static NATypeVtbl __Hoge__NATypeVtbl = {
    __HogeInit,
    __HogeDestroy,
    __HogeHash,
    __HogeEqualTo,
    __HogeCompare,
    __HogeCopy,
    __HogeDescription,
};

static HogeVtbl __Hoge__HogeVtbl = {
    __HogeSayHoge
};

static NAVtblEntry __Hoge__vEntries[] = {
    {NATypeID, &__Hoge__NATypeVtbl},
    {HogeID, &__Hoge__HogeVtbl},
    {NULL, NULL},
};

NAClass HogeClass = {
    HogeID,
    sizeof(Hoge),
    __Hoge__vEntries,
};

int main(int argc, char **argv)
{
    Hoge *hoge1 = NATypeNew(Hoge, 1);
    Hoge *hoge2 = NATypeNew(Hoge, 2);

    printf("hoge1:hash %d\n", NAHash(hoge1));
    printf("hoge2:hash %d\n", NAHash(hoge2));
    printf("hoge1:hoge2:equal %d\n", NAEqual(hoge1, hoge2));
    printf("hoge1:hoge2:compare %d\n", NACompare(hoge1, hoge2));

    Hoge *copied1 = NACopy(hoge1);
    printf("hoge1:copied1:equal %d\n", NAEqual(hoge1, copied1));
    printf("hoge1:copied1:compare %d\n", NACompare(hoge1, copied1));

    CFStringRef cfString;
    char buf[1024];

    cfString = NADescription(hoge1);
    CFStringGetCString(cfString, buf, sizeof(buf), kCFStringEncodingUTF8);
    printf("hoge1:description %s\n", buf);
    CFRelease(cfString);

    CFMutableArrayRef cfArray = CFArrayCreateMutable(NULL, 0, &NACFArrayCallBacks);
    CFArrayAppendValue(cfArray, hoge1);
    CFArrayAppendValue(cfArray, hoge2);

    cfString = CFCopyDescription(cfArray);
    CFStringGetCString(cfString, buf, sizeof(buf), kCFStringEncodingUTF8);
    printf("cfArray:description %s\n", buf);
    CFRelease(cfString);

    CFRelease(cfArray);

    NARelease(hoge1);
    NARelease(hoge2);
    NARelease(copied1);

    return 0;
}

