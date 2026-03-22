#include "WindowManager.h"
#include "Window.h"
#include "NASet.h"
#include <stdlib.h>

struct _WindowManager {
    NASet *windows;
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
}

void WindowManagerRemoveWindow(WindowManager *self, Window *window)
{
    NASetRemove(self->windows, window);
}

void WindowManagerDisplayIfNeeded(WindowManager *self)
{
    NASetTraverse(self->windows, WindowDisplayIfNeeded);
}
