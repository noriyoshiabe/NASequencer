#include "Argument.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static struct option _options[] = {
    { "sound-font", required_argument, NULL, 's' },
    { "gain", required_argument, NULL, 'g' },
    { NULL, 0, NULL, 0 }
};

extern Argument *ArgumentParse(int argc, char **argv)
{
    Argument *self = calloc(1, sizeof(Argument));

    int opt;
    
    while (-1 != (opt = getopt_long(argc, argv, "s:g:", _options, NULL))) {
        switch (opt) {
        case 's':
            self->soundSource = strdup(optarg);
            break;
        case 'g':
            self->gain = atoi(optarg);
            break;
        case '?':
            self->error = malloc(64);
            sprintf(self->error, "Unknown option -%c.", opt);
            return self;
        }
    }

    if (optind < argc) {
        self->filepath = strdup(argv[optind]);
        FILE *fp = fopen(self->filepath, "r");
        if (!fp) {
            self->error = malloc(64);
            snprintf(self->error, 64, "File not found. %s", self->filepath);
            return self;
        }
        fclose(fp);
    }

    char buffer[64];
    sprintf(buffer, "");

    if (!self->filepath) {
        strcat(buffer, "No input source file. ");
    }

    if (!self->soundSource) {
        strcat(buffer, "No sound sources.");
    }

    if (0 < strlen(buffer)) {
        self->error = strdup(buffer);
    }

    return self;
}

extern void ArgumentDestroy(Argument *self)
{
    if (self->filepath) {
        free(self->filepath);
    }

    if (self->soundSource) {
        free(self->soundSource);
    }

    if (self->error) {
        free(self->error);
    }

    free(self);
}
