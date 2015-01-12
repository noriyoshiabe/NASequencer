#pragma once

typedef struct _ASTNode {
    int type;
    union {
        int i;
        float f;
        char *s;
    } v;
    struct _ASTNode *childlen;
    struct _ASTNode *next;
} ASTNode;
