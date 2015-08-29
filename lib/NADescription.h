#pragma once

typedef void (*NADescription)(void *value, char *buffer, int length);
extern void NADescriptionAddress(void *value, char *buffer, int length);
extern void NADescriptionCString(void *value, char *buffer, int length);
