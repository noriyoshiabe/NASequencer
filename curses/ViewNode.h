#pragma once

#include "View.h"
#include "Window.h"

#include <stdarg.h>

typedef void View;
typedef struct _ViewNode ViewNode;

extern ViewNode *ViewNodeCreate(View *owner);
extern void ViewNodeDestroy(ViewNode *self);
extern Window *ViewNodeGetWindow(ViewNode *self);
extern void ViewNodeSetWindow(ViewNode *self, Window *window);
extern View *ViewNodeGetParent(ViewNode *self);
extern void ViewNodeAppendChild(ViewNode *self, View *child);
extern void ViewNodeRemoveChild(ViewNode *self, View *child);
extern Rect ViewNodeGetFrame(ViewNode *self);
extern void ViewNodeSetFrame(ViewNode *self, Rect frame);
extern bool ViewNodeGetHidden(ViewNode *self);
extern void ViewNodeSetHidden(ViewNode *self, bool hidden);
extern void ViewNodeInvalidate(ViewNode *self);
extern void ViewNodeDisplay(ViewNode *self);
extern bool ViewNodeIsDrawing(ViewNode *self);
extern void ViewNodeSetAttr(ViewNode *self, int attrs);
extern void ViewNodeBecomeKeyView(ViewNode *self);
extern bool ViewNodeIsKeyView(ViewNode *self);
extern void ViewNodePrintf(ViewNode *self, int x, int y, const char *fmt, va_list argList);
