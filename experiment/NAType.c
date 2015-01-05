#include <NAType.h>
#include <NAString.h>

char NATypeID[] = "NAType";

void *NATypeInitEmpty(void *self, ...)
{
    return self;
}

void NATypeDestroyDefault(void *self)
{
}

uint32_t NATypeHashDefault(const void *self)
{
    return (uint32_t)self >> 2;
}

bool NATypeEqualDefault(const void *self, const void *to)
{
    return NAHash(self) == NAHash(to);
}

int NATypeCompareDefault(const void *self, const void *to)
{
    return (int)self - (int)to;
}
