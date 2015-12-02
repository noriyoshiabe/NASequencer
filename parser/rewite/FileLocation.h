#pragma once

typedef struct _FileLocation {
    char *filepath;
    int line;
    int column;
} FileLocation;
