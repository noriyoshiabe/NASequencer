#pragma once

#include <stdio.h>

typedef void (*NADescription)(void *value, FILE *stream);
extern void NADescriptionAddress(void *value, FILE *stream);
extern void NADescriptionCString(void *value, FILE *stream);
