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
#define BOOL(ast, name) #name, NACStringFromBoolean(ast->name)
#define STRING(ast, name) #name, ast->name ? ast->name : "(null)"

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

static void visitFileIdentification(void *self, ASTFileIdentification *ast)
{
    dump(self, ast, STRING(ast, identifier), INTEGER(ast, major), INTEGER(ast, minor), NULL);
}

Analyzer *ABCASTDumperCreate(ParseContext *context)
{
    ABCASTDumper *self = calloc(1, sizeof(ABCASTDumper));

    self->visitor.visitRoot = visitRoot;
    self->visitor.visitFileIdentification = visitFileIdentification;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    return &self->analyzer;
}
