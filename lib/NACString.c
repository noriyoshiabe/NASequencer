#include "NACString.h"

#include <stdio.h>

#undef NACStringFromInteger
#undef NACStringFromFloat
#undef NACStringDuplicate

char *NACStringFromInteger(int i, char *buffer, int size)
{
    snprintf(buffer, size, "%d", i);
    return buffer;
}

char *NACStringFromFloat(float f, int point, char *buffer, int size)
{
    snprintf(buffer, size, "%.*f", point, f);
    return buffer;
}

char *NACStringDuplicate(char *str, char *buffer)
{
    strcpy(buffer, str);
    return buffer;
}
