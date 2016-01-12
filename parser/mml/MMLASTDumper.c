#include "MMLASTDumper.h"
#include "MMLAST.h"
#include <NACString.h>
#include <NALog.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct _MMLASTDumper {
    ASTVisitor visitor;
    Analyzer analyzer;
    int indent;
} MMLASTDumper;

static Node *process(void *self, Node *node)
{
    if (__IsDebug__) {
        node->accept(node, self);
    }
    //return NodeRetain(node);
    return NULL;
}

static void destroy(void *self)
{
    free(self);
}

static void dump(MMLASTDumper *self, void *_node, ...)
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
#define STRING(ast, name) #name, ast->name ? ast->name : "(null)"

static void visitRoot(void *_self, ASTRoot *ast)
{
    MMLASTDumper *self = _self;

    dump(self, ast, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

Analyzer *MMLASTDumperCreate(ParseContext *context)
{
    MMLASTDumper *self = calloc(1, sizeof(MMLASTDumper));

    self->visitor.visitRoot = visitRoot;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    return &self->analyzer;
}
