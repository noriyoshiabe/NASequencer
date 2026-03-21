#pragma once

#include "View.h"
#include "Window.h"

#include <stdarg.h>

typedef void View;
typedef struct _ViewNode ViewNode;

extern ViewNode *ViewNodeCreate(View *owner);
extern Window *ViewNodeGetWindow(ViewNode *self);
extern void ViewNodeSetWindow(ViewNode *self, Window *window);
extern View *ViewNodeGetParent(ViewNode *self);
extern void ViewNodeAppendChild(ViewNode *self, View *child);
extern void ViewNodeRemoveChild(ViewNode *self, View *child);
extern Rect ViewNodeGetFrame(ViewNode *self);
extern void ViewNodeSetFrame(ViewNode *self, Rect frame);
extern void ViewNodeInvalidate(ViewNode *self);
extern bool ViewNodeIsDrawing(ViewNode *self);
extern void ViewNodePrintf(ViewNode *self, int x, int y, const char *fmt, va_list argList);
extern void ViewNodeDestroy(ViewNode *self);
