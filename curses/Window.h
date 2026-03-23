#pragma once

#include "Rect.h"
#include "Color.h"

#include <stdbool.h>

typedef struct _Window Window;
typedef struct _WindowManager WindowManager;
typedef void View;

extern Window *WindowCreate(Rect rect);
extern void WindowDestroy(Window *self);
extern void WindowAppendView(Window *self, View *view);
extern void WindowRemoveView(Window *self, View *view);
extern void WindowSetKeyView(Window *self, View *view);
extern bool WindowIsKeyView(Window *self, View *view);
extern bool WindowDispatchKeyEvent(Window *self, int code);
extern void WindowMarkViewAsDirty(Window *self, View *view);
extern void WindowSetColor(Window *self, Color color);
extern void WindowPrint(Window *self, int x, int y, const char *str);
extern void WindowRefresh(Window *self);
extern void WindowDisplayIfNeeded(Window *self);
