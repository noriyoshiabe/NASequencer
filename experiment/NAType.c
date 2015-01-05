#include <NAType.h>
#include <NAString.h>

int NATypeID;

void *__NATypeInit(void *self, ...)
{
    return self;
}

void __NATypeDestroy(void *self)
{
}

uint32_t __NATypeHash(const void *self)
{
    return (uint32_t)self >> 2;
}

bool __NATypeEqual(const void *self, const void *to)
{
    return NAHash(self) == NAHash(to);
}

int __NATypeCompare(const void *self, const void *to)
{
    return (int)self - (int)to;
}
