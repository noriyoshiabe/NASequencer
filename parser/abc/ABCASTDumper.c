#include "ABCASTDumper.h"
#include "ABCAST.h"
#include <NACString.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct _ABCASTDumper {
    ASTVisitor visitor;
    Analyzer analyzer;
    int indent;
} ABCASTDumper;

static Node *process(void *self, Node *node)
{
    node->accept(node, self);
    //return NodeRetain(node);
    return NULL;
}

static void destroy(void *self)
{
    free(self);
}

static void dump(ABCASTDumper *self, void *_node, ...)
{
    Node *node = _node;

    printf("%*s", self->indent, "");
    printf("[%s]", node->type);

    va_list argList;
    va_start(argList, _node);

    const char *str;
    int i = 0;
    while ((str = va_arg(argList, const char *))) {
        printf("%c", 0 == i % 2 ? ' ' : '=');
        printf("%s", str);
        ++i;
    }

    va_end(argList);

    printf(" at %s:%d:%d\n", node->location.filepath, node->location.line, node->location.column);
}

#define INTEGER(ast, name) #name, NACStringFromInteger(ast->name)
#define FLOAT(ast, name) #name, NACStringFromFloat(ast->name, 2)
#define CHAR(ast, name) #name, NACStringFromChar(ast->name)
#define STRING(ast, name) #name, ast->name

static void visitRoot(void *_self, ASTRoot *ast)
{
    ABCASTDumper *self = _self;

    dump(self, ast, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitStringInformation(void *self, ASTStringInformation *ast)
{
    dump(self, ast, CHAR(ast, field), STRING(ast, string), NULL);
}

static void visitVersion(void *self, ASTVersion *ast)
{
    dump(self, ast, STRING(ast, versionString), NULL);
}

static void visitReferenceNumber(void *self, ASTReferenceNumber *ast)
{
    dump(self, ast, INTEGER(ast, number), NULL);
}

static void visitTitle(void *self, ASTTitle *ast)
{
    dump(self, ast, STRING(ast, title), NULL);
}

static void visitKey(void *_self, ASTKey *ast)
{
    ABCASTDumper *self = _self;

    dump(self, ast, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitKeyParam(void *self, ASTKeyParam *ast)
{
    switch (ast->type) {
    case KeyTonic:
    case KeyMode:
    case KeyAccidental:
    case Clef:
    case Middle:
        dump(self, ast, "type", ASTKeyParamType2String(ast->type), STRING(ast, string), NULL);
        break;
    case Transpose:
    case Octave:
    case StaffLines:
        dump(self, ast, "type", ASTKeyParamType2String(ast->type), INTEGER(ast, intValue), NULL);
        break;
    }
}

static void visitNote(void *self, ASTNote *ast)
{
    dump(self, ast, STRING(ast, noteString), NULL);
}

static void visitLineBreak(void *self, ASTLineBreak *ast)
{
    dump(self, ast, NULL);
}

static void visitInclude(void *_self, ASTInclude *ast)
{
    ABCASTDumper *self = _self;

    dump(self, ast, STRING(ast, filepath), NULL);
    self->indent += 4;

    if (ast->root) {
        ast->root->accept(ast->root, self);
    }

    self->indent -= 4;
}

Analyzer *ABCASTDumperCreate(ParseContext *context)
{
    ABCASTDumper *self = calloc(1, sizeof(ABCASTDumper));

    self->visitor.visitRoot = visitRoot;
    self->visitor.visitVersion = visitVersion;
    self->visitor.visitStringInformation = visitStringInformation;
    self->visitor.visitTitle = visitTitle;
    self->visitor.visitReferenceNumber = visitReferenceNumber;
    self->visitor.visitNote = visitNote;
    self->visitor.visitKey = visitKey;
    self->visitor.visitKeyParam = visitKeyParam;
    self->visitor.visitLineBreak = visitLineBreak;
    self->visitor.visitInclude = visitInclude;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    return &self->analyzer;
}
