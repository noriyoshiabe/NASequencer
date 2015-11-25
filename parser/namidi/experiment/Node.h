#pragma once

typedef struct _Node {
    struct Node *next;
} Node;

typedef struct _NodeList {
    Node *head;
    Node *tail;
} NodeList;

extern NodeList *NodeListCreate();
extern NodeList *NodeListAppend(NodeList *self, Node *node);
extern void NodeListTraverse(NodeList *self, void (*)(void *));
