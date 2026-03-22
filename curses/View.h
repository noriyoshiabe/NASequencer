#pragma once

#include "Interface.h"
#include "Window.h"
#include "Rect.h"
#include "NAArray.h"
#include "Color.h"

extern const IID IIDView;

typedef void View;
typedef struct _ViewNode ViewNode;

typedef struct {
    ViewNode *(*getNode)(View *self);
    void (*draw)(View *self, Size size);
    void (*destroy)(View *self);
} ViewVtbl, *IView;

static inline ViewNode *ViewGetNode(View *self)
{
    return QueryInterface(self, IIDView, IView)->getNode(self);
}

static inline void ViewDraw(View *self, Size size)
{
    QueryInterface(self, IIDView, IView)->draw(self, size);
}

static inline void ViewDestroy(View *self)
{
    QueryInterface(self, IIDView, IView)->destroy(self);
}

extern Window *ViewGetWindow(View *self);
extern void ViewSetWindow(View *self, Window *window);
extern View *ViewGetParent(View *self);
extern void ViewAppendChild(View *self, View *child);
extern void ViewRemoveChild(View *self, View *child);
extern Rect ViewGetFrame(View *self);
extern void ViewSetFrame(View *self, Rect frame);
extern bool ViewGetHidden(View *self);
extern void ViewSetHidden(View *self, bool hidden);
extern void ViewInvalidate(View *self);
extern void ViewDisplay(View *self);
extern void ViewSetColor(View *self, Color color);
extern void ViewPrintf(View *self, int x, int y, const char *fmt, ...);
