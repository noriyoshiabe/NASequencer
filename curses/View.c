#include "View.h"
#include "ViewNode.h"
#include <stdarg.h>

const IID IIDView = InterfaceID("View");

Window *ViewGetWindow(View *self)
{
    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    return ViewNodeGetWindow(node);
}

void ViewSetWindow(View *self, Window *window)
{
    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    ViewNodeSetWindow(node, window);
}

View *ViewGetParent(View *self)
{
    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    return ViewNodeGetParent(node);
}

void ViewAppendChild(View *self, View *child)
{
    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    ViewNodeAppendChild(node, child);
}

void ViewRemoveChild(View *self, View *child)
{
    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    ViewNodeRemoveChild(node, child);
}

Rect ViewGetFrame(View *self)
{
    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    return ViewNodeGetFrame(node);
}

void ViewSetFrame(View *self, Rect frame)
{
    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    ViewNodeSetFrame(node, frame);
}

void ViewInvalidate(View *self)
{
    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    ViewNodeInvalidate(node);
}

void ViewDisplay(View *self)
{
    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    ViewNodeDisplay(node);
}

void ViewPrintf(View *self, int x, int y, const char *fmt, ...)
{
    va_list argList;
    va_start(argList, fmt);

    ViewNode *node = QueryInterface(self, IIDView, IView)->getNode(self);
    ViewNodePrintf(node, x, y, fmt, argList);

    va_end(argList);
}
