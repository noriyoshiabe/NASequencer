#pragma once

typedef struct {
    char *filepath;
    char *soundSources[8];
    int gain;
    char *error;
} Argument;

extern Argument *ArgumentParse(int argc, char **argv);
extern void ArgumentDestroy(Argument *self);
