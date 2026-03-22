#pragma once

#include "Rect.h"
#include "KeyHandler.h"

typedef struct _Window Window;
typedef struct _WindowManager WindowManager;
typedef void View;

extern Window *WindowCreate(Rect rect);
extern void WindowDestroy(Window *self);
extern void WindowAppendView(Window *self, View *view);
extern void WindowRemoveView(Window *self, View *view);
extern void WindowMarkViewAsDirty(Window *self, View *view);
extern void WindowPrint(Window *self, int x, int y, const char *str);
extern void WindowRefresh(Window *self);
extern void WindowDisplayIfNeeded(Window *self);
