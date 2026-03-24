#pragma once

typedef struct {
    char *filepath;
    char *soundSource;
    int gain;
    char *error;
} Argument;

extern Argument *ArgumentParse(int argc, char **argv);
extern void ArgumentDestroy(Argument *self);
