#include "NADescription.h"

#include <stdint.h>

void NADescriptionAddress(void *value, FILE *stream)
{
    fprintf(stream, "<%08X>", (uint32_t)value);
}

void NADescriptionCString(void *value, FILE *stream)
{
    fprintf(stream, "%s", (char *)value);
}

void NADescriptionCInteger(void *value, FILE *stream)
{
    fprintf(stream, "%d", *((int *)value));
}
