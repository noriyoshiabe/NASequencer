#pragma once

#include "Node.h"

typedef struct _ASTRoot {
    Node node;
} ASTRoot;

typedef struct _ASTFileIdentification {
    Node node;
    char *identifier;
    int major;
    int minor;
} ASTFileIdentification;

typedef struct _ASTVisitor {
    void (*visitRoot)(void *self, ASTRoot *ast);
    void (*visitFileIdentification)(void *self, ASTFileIdentification *ast);
} ASTVisitor;

extern ASTRoot *ABCASTRootCreate(FileLocation *location);
extern ASTFileIdentification *ABCASTFileIdentificationCreate(FileLocation *location);
