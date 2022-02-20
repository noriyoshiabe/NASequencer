#include "NAHash.h"

uint32_t NAHashAddress(void *value)
{
    return (uint32_t)((uintptr_t)value >> 2);
}

uint32_t NAHashCString(void *value)
{
    uint32_t h = 0;
    char *p = value;
    while (*p) {
        h = 31 * h + *(p++);
    }
    return h;
}

uint32_t NAHashCInteger(void *value)
{
    return *((uint32_t *)value);
}
