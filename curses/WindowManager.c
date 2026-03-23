#include "WindowManager.h"
#include "Window.h"
#include "NASet.h"
#include <stdlib.h>

struct _WindowManager {
    NASet *windows;
    Window *keyWindow;
};

static WindowManager *_sharedInstance = NULL;

static WindowManager *WindowManagerCreate()
{
    WindowManager *self = calloc(1, sizeof(WindowManager));
    self->windows = NASetCreate(NULL, NULL);
    return self;
}

static void WindowManagerDestroy()
{
    WindowManager *self = _sharedInstance;
    NASetDestroy(self->windows);
    free(self);
}

WindowManager *WindowManagerSharedInstance()
{
    if (!_sharedInstance) {
        _sharedInstance = WindowManagerCreate();
        atexit(WindowManagerDestroy);
    }
    return _sharedInstance;
}

void WindowManagerAppendWindow(WindowManager *self, Window *window)
{
    NASetAdd(self->windows, window);
    self->keyWindow = window;
}

void WindowManagerRemoveWindow(WindowManager *self, Window *window)
{
    NASetRemove(self->windows, window);

    if (self->keyWindow == window) {
        self->keyWindow = NULL;
    }
}

bool WindowManagerDispatchKeyEvent(WindowManager *self, int code)
{
    if (self->keyWindow) {
        return WindowDispatchKeyEvent(self->keyWindow, code);
    }

    return false;
}

void WindowManagerDisplayIfNeeded(WindowManager *self)
{
    NASetTraverse(self->windows, WindowDisplayIfNeeded);
}
