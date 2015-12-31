#include "NACInteger.h"

#include <stdlib.h>

int *NACIntegerFromInteger(int i)
{
    int *value = malloc(sizeof(int));
    *value = i;
    return value;
}
