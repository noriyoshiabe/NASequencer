#include "NADescription.h"

#include <stdio.h>
#include <stdint.h>

void NADescriptionAddress(void *value, char *buffer, int length)
{
    snprintf(buffer, length, "<%08X>", (uint32_t)value);
}

void NADescriptionCString(void *value, char *buffer, int length)
{
    snprintf(buffer, length, "%s", (char *)value);
}
