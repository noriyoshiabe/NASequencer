#include "ViewNode.h"
#include <stdlib.h>
#include <string.h>

struct _ViewNode {
    View *owner;
    View *parent;
    NAArray *children;
    Rect frame;
    Window *window;
    bool isDrawing;
    bool hidden;
};

static Point ViewNodeOffsetInRootView(ViewNode *self);

ViewNode *ViewNodeCreate(View *owner)
{
    ViewNode *self = calloc(1, sizeof(ViewNode));
    self->owner = owner;
    self->children = NAArrayCreate(4, NULL);
    return self;
}

void ViewNodeDestroy(ViewNode *self)
{
    if (self->window) {
        WindowRemoveView(self->window, self->owner);
    }

    NAIterator *iterator = NAArrayGetIterator(self->children);
    while (iterator->hasNext(iterator)) {
        View *child = iterator->next(iterator);
        ViewDestroy(child);
    }

    NAArrayDestroy(self->children);
    free(self);
}

Window *ViewNodeGetWindow(ViewNode *self)
{
    return self->window;
}

void ViewNodeSetWindow(ViewNode *self, Window *window)
{
    self->window = window;

    if (self->window) {
        WindowAppendView(self->window, self->owner);
    }

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

    if (childNode->window) {
        WindowAppendView(childNode->window, childNode->owner);
    }

    childNode->parent = self->owner;

    NAArrayAppend(self->children, child);
}

void ViewNodeRemoveChild(ViewNode *self, View *child)
{
    int index = NAArrayFindFirstIndex(self->children, child, NAArrayAddressComparator);
    NAArrayRemoveAt(self->children, index);

    ViewNode *childNode = ViewGetNode(child);
    if (childNode->window) {
        WindowRemoveView(childNode->window, childNode->owner);
        childNode->window = NULL;
    }
    childNode->parent = NULL;
};

Rect ViewNodeGetFrame(ViewNode *self)
{
    return self->frame;
}

void ViewNodeSetFrame(ViewNode *self, Rect frame)
{
    self->frame = frame;
}

bool ViewNodeGetHidden(ViewNode *self)
{
    return self->hidden;
}

void ViewNodeSetHidden(ViewNode *self, bool hidden)
{
    self->hidden = hidden;
}

void ViewNodeInvalidate(ViewNode *self)
{
    WindowMarkViewAsDirty(self->window, self->owner);
}

void ViewNodeDisplay(ViewNode *self)
{
    if (self->hidden)
        return;

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
    NAArrayTraverse(self->children, ViewDisplay);

    self->isDrawing = false;

    if (!isParentDrawing) {
        WindowRefresh(self->window);
    }
}

bool ViewNodeIsDrawing(ViewNode *node)
{
    return node->isDrawing;
}

void ViewNodeSetColor(ViewNode *self, Color color)
{
    WindowSetColor(self->window, color);
}

void ViewNodeBecomeKeyView(ViewNode *self)
{
    WindowSetKeyView(self->window, self->owner);
}

bool ViewNodeIsKeyView(ViewNode *self)
{
    return WindowIsKeyView(self->window, self->owner);
}

void ViewNodePrintf(ViewNode *self, int x, int y, const char *fmt, va_list argList)
{
    int maxLength = self->frame.size.width - x;
    char line[maxLength + 1];
    vsnprintf(line, maxLength + 1, fmt, argList);

    Point offset = ViewNodeOffsetInRootView(self);
    WindowPrint(self->window, offset.x + x, offset.y + y, line);
}

static Point ViewNodeOffsetInRootView(ViewNode *self)
{
    Point point = self->frame.point;

    View *ancestor = self->parent;
    while (ancestor) {
        Rect ancestorFrame = ViewGetFrame(ancestor);
        point.x += ancestorFrame.point.x;
        point.y += ancestorFrame.point.y;
        ancestor = ViewGetParent(ancestor);
    }

    return point;
}
