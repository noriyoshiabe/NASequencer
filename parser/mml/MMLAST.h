#pragma once

#include "Node.h"

typedef struct _ASTRoot {
    Node node;
} ASTRoot;

typedef struct _ASTInclude {
    Node node;
    char *filepath;
    char *fullpath;
    Node *root;
} ASTInclude;

typedef struct _ASTVisitor {
    void (*visitRoot)(void *self, ASTRoot *ast);
    void (*visitInclude)(void *self, ASTInclude *ast);
} ASTVisitor;

extern ASTRoot *MMLASTRootCreate(FileLocation *location);
