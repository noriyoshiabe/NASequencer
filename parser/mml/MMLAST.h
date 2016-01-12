#pragma once

#include "Node.h"

typedef struct _ASTRoot {
    Node node;
} ASTRoot;

typedef struct _ASTVisitor {
    void (*visitRoot)(void *self, ASTRoot *ast);
} ASTVisitor;

extern ASTRoot *MMLASTRootCreate(FileLocation *location);
