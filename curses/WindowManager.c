#include "WindowManager.h"
#include "Window.h"
#include "NAArray.h"
#include <stdlib.h>
#include <ncurses.h>

struct _WindowManager {
    NAArray *windows;
    Window *keyWindow;
};

static void WindowManagerRefresh(WindowManager *self);

static WindowManager *_sharedInstance = NULL;

static WindowManager *WindowManagerCreate()
{
    WindowManager *self = calloc(1, sizeof(WindowManager));
    self->windows = NAArrayCreate(4, NULL);
    return self;
}

static void WindowManagerDestroy()
{
    WindowManager *self = _sharedInstance;
    NAArrayDestroy(self->windows);
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
    NAArrayAppend(self->windows, window);
    self->keyWindow = window;
    WindowManagerRefresh(self);
}

void WindowManagerRemoveWindow(WindowManager *self, Window *window)
{
    int index = NAArrayFindFirstIndex(self->windows, window, NAArrayAddressComparator);
    NAArrayRemoveAt(self->windows, index);

    if (self->keyWindow == window) {
        self->keyWindow = NAArrayGetValueAt(self->windows, NAArrayCount(self->windows) - 1);
    }

    WindowManagerRefresh(self);
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
    bool displayed = false;

    NAIterator *iterator = NAArrayGetIterator(self->windows);
    while (iterator->hasNext(iterator)) {
        Window *window = iterator->next(iterator);

        if (displayed) {
            WindowTouch(window);
            WindowNOutRefresh(window);
        } else {
            displayed = WindowDisplayIfNeeded(window);
        }
    }

    if (displayed) {
        doupdate();
    }
}

static void WindowManagerRefresh(WindowManager *self)
{
    NAIterator *iterator = NAArrayGetIterator(self->windows);
    while (iterator->hasNext(iterator)) {
        Window *window = iterator->next(iterator);
        WindowTouch(window);
        WindowNOutRefresh(window);
    }

    doupdate();
}
