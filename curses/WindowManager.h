#pragma once

typedef struct _WindowManager WindowManager;
typedef struct _Window Window;

extern WindowManager *WindowManagerSharedInstance();
extern void WindowManagerAppendWindow(WindowManager *self, Window *window);
extern void WindowManagerRemoveWindow(WindowManager *self, Window *window);
extern void WindowManagerDisplayIfNeeded(WindowManager *self);
