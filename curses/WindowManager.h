#pragma once

#include <stdbool.h>

typedef struct _WindowManager WindowManager;
typedef struct _Window Window;

extern WindowManager *WindowManagerSharedInstance();
extern void WindowManagerAppendWindow(WindowManager *self, Window *window);
extern void WindowManagerRemoveWindow(WindowManager *self, Window *window);
extern bool WindowManagerDispatchKeyEvent(WindowManager *self, int code);
extern void WindowManagerDisplayIfNeeded(WindowManager *self);
