#pragma once

#include "Rect.h"
#include "KeyHandler.h"

typedef struct _Window Window;

extern Window *WindowCreate(Rect rect);
extern void WindowDestroy(Window *self);
extern void WindowPrint(Window *self, int x, int y, const char *str);
extern void WindowRefresh(Window *self);
