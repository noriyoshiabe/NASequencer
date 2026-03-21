#include "ViewNode.h"
#include <stdlib.h>
#include <string.h>

struct _ViewNode {
    View *owner;
    View *parent;
    NAArray *children;
    Rect frame;
    Window *window;
    char *buffer;
    bool isDrawing;
};

static Point ViewNodeOffsetInRootView(ViewNode *self);

ViewNode *ViewNodeCreate(View *owner)
{
    ViewNode *self = calloc(1, sizeof(ViewNode));
    self->owner = owner;
    self->children = NAArrayCreate(4, NULL);
    self->buffer = calloc(1, 1);
    return self;
};

Window *ViewNodeGetWindow(ViewNode *self)
{
    return self->window;
}

void ViewNodeSetWindow(ViewNode *self, Window *window)
{
    self->window = window;

    NAIterator *iterator = NAArrayGetIterator(self->children);
    while (iterator->hasNext(iterator)) {
        View *child = iterator->next(iterator);
        ViewSetWindow(child, window);
    }
}

View *ViewNodeGetParent(ViewNode *self)
{
    return self->parent;
}

void ViewNodeAppendChild(ViewNode *self, View *child)
{
    ViewNode *childNode = ViewGetNode(child);
    if (childNode->parent) {
        ViewNodeRemoveChild(childNode, child);
    }

    childNode->window = self->window;
    childNode->parent = self->parent;

    NAArrayAppend(self->children, child);
}

void ViewNodeRemoveChild(ViewNode *self, View *child)
{
    int index = NAArrayFindFirstIndex(self->children, child, NAArrayAddressComparator);
    NAArrayRemoveAt(self->children, index);

    ViewNode *childNode = ViewGetNode(child);
    childNode->window = NULL;
    childNode->parent = NULL;
};

Rect ViewNodeGetFrame(ViewNode *self)
{
    return self->frame;
}

void ViewNodeSetFrame(ViewNode *self, Rect frame)
{
    self->frame = frame;
    self->buffer = realloc(self->buffer, frame.size.width * frame.size.height + 1);
}

void ViewNodeInvalidate(ViewNode *self)
{
    bool isParentDrawing = self->parent && ViewNodeIsDrawing(ViewGetNode(self->parent));

    if (!isParentDrawing) {
        char line[self->frame.size.width + 1];
        memset(line, ' ', self->frame.size.width);
        line[self->frame.size.width] = '\0';

        Point offset = ViewNodeOffsetInRootView(self);
        for (int i = 0; i < self->frame.size.height; ++i) {
            WindowPrint(self->window, 0, offset.y + i, line);
        }
    }

    self->isDrawing = true;

    ViewDraw(self->owner, self->frame.size);
    NAArrayTraverse(self->children, ViewInvalidate);

    self->isDrawing = false;

    if (!isParentDrawing) {
        WindowRefresh(self->window);
    }
}

void ViewNodePrintf(ViewNode *self, int x, int y, const char *fmt, va_list argList)
{
    int maxLength = self->frame.size.width - x;
    char line[maxLength + 1];
    vsnprintf(line, maxLength + 1, fmt, argList);

    Point offset = ViewNodeOffsetInRootView(self);
    WindowPrint(self->window, offset.x + x, offset.y + y, line);
}

bool ViewNodeIsDrawing(ViewNode *node)
{
    return node->isDrawing;
}

void ViewNodeDestroy(ViewNode *self)
{
    NAIterator *iterator = NAArrayGetIterator(self->children);
    while (iterator->hasNext(iterator)) {
        View *child = iterator->next(iterator);
        ViewDestroy(child);
    }

    NAArrayDestroy(self->children);
    free(self);
}

static Point ViewNodeOffsetInRootView(ViewNode *self)
{
    Point point = self->frame.point;

    View *ancestor = self->parent;
    while (ancestor) {
        ancestor = ViewGetParent(ancestor);
        Rect ancestorFrame = ViewGetFrame(ancestor);
        point.x += ancestorFrame.point.x;
        point.y += ancestorFrame.point.y;
    }

    return point;
}
