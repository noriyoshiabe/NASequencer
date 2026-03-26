#include "Window.h"
#include "WindowManager.h"
#include "Debug.h"
#include "View.h"
#include "KeyHandler.h"
#include "NASet.h"
#include "NAMessageQ.h"

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

struct _Window {
    WINDOW *window;
    View *rootView;
    View *keyView;
    NASet *views;
    NAMessageQ *msgQ;
};

static const int WindowMessageDirtyView = 1;

Window *WindowCreate(Rect rect)
{
    Window *self = calloc(1, sizeof(Window));
    self->views = NASetCreate(NULL, NULL);
    self->msgQ = NAMessageQCreate(32);
    self->window = newwin(rect.size.height, rect.size.width, rect.point.y, rect.point.x);

    WindowManager *windowManager = WindowManagerSharedInstance();
    WindowManagerAppendWindow(windowManager, self);

    return self;
}

void WindowDestroy(Window *self)
{
    WindowManager *windowManager = WindowManagerSharedInstance();
    WindowManagerRemoveWindow(windowManager, self);

    delwin(self->window);
    free(self);
}

void WindowSetBox(Window *self)
{
    box(self->window, 0, 0);
}

void WindowAppendView(Window *self, View *view)
{
    if (0 == NASetCount(self->views)) {
        self->rootView = view;
    }

    NASetAdd(self->views, view);
}

void WindowRemoveView(Window *self, View *view)
{
    NASetRemove(self->views, view);

    if (self->keyView == view) {
        self->keyView = self->rootView;
    }
}

bool WindowDispatchKeyEvent(Window *self, int code)
{
    KeyHandler *keyHandler = self->keyView;

    while (keyHandler) {
        if (KeyHandlerHandleKeyEvent(keyHandler, code)) {
            return true;
        }
        keyHandler = KeyHandlerGetNextKeyHandler(keyHandler);
    }

    return false;
}

void WindowSetKeyView(Window *self, View *view)
{
    self->keyView = view;
}

bool WindowIsKeyView(Window *self, View *view)
{
    return self->keyView == view;
}

void WindowMarkViewAsDirty(Window *self, View *view)
{
    NAMessageQPost(self->msgQ, WindowMessageDirtyView, view);
}

void WindowSetAttr(Window *self, int attrs)
{
    wattrset(self->window, attrs);
}

void WindowPrint(Window *self, int x, int y, const char *str)
{
    mvwprintw(self->window, y, x, str);
}

void WindowRefresh(Window *self)
{
    wrefresh(self->window);
}

bool WindowDisplayIfNeeded(Window *self)
{
    NAMessage msg;
    bool displayed = false;

    while (NAMessageQPeek(self->msgQ, &msg)) {
        switch (msg.kind) {
        case WindowMessageDirtyView:
            if (NASetContains(self->views, msg.data)) {
                ViewDisplay(msg.data);
                displayed = true;
            }
            break;
        }
    }

    return displayed;
}

void WindowTouch(Window *self)
{
    touchwin(self->window);
}

void WindowNOutRefresh(Window *self)
{
    wnoutrefresh(self->window);
}
