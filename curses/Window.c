#include "Window.h"
#include "WindowManager.h"
#include "Debug.h"
#include "View.h"
#include "NASet.h"
#include "NAMessageQ.h"

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

struct _Window {
    const InterfaceVtbl *vtbl;
    WINDOW *window;
    KeyHandler *nextKeyHandler;
    NASet *views;
    NAMessageQ *msgQ;
};

static const int WindowMessageDirtyView = 1;

static bool WindowOnKeyEvent(KeyHandler *self, int code);
static void WindowSetNextKeyHandler(KeyHandler *self, KeyHandler *keyHandler);

static const KeyHandlerVtbl WindowKeyHandlerVtbl = {
    .onKeyEvent = WindowOnKeyEvent,
    .setNextKeyHandler = WindowSetNextKeyHandler,
};

static void *WindowQueryInteface(void *self, IID iid)
{
    if (iid == IIDKeyHandler)
        return (void*)&WindowKeyHandlerVtbl;
    return NULL;
}

static const InterfaceVtbl WindowInterfaceVtbl = {
    .queryInterface = WindowQueryInteface,
};

Window *WindowCreate(Rect rect)
{
    Window *self = calloc(1, sizeof(Window));
    self->vtbl = &WindowInterfaceVtbl;
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

void WindowAppendView(Window *self, View *view)
{
    NASetAdd(self->views, view);
}

void WindowRemoveView(Window *self, View *view)
{
    NASetRemove(self->views, view);
}

void WindowMarkViewAsDirty(Window *self, View *view)
{
    NAMessageQPost(self->msgQ, WindowMessageDirtyView, view);
}

void WindowSetColor(Window *self, Color color)
{
    wattrset(self->window, color == ColorDefault ? A_NORMAL : COLOR_PAIR(color));
}

void WindowPrint(Window *self, int x, int y, const char *str)
{
    mvwprintw(self->window, y, x, str);
}

void WindowRefresh(Window *self)
{
    wrefresh(self->window);
}

void WindowDisplayIfNeeded(Window *self)
{
    NAMessage msg;

    while (NAMessageQPeek(self->msgQ, &msg)) {
        switch (msg.kind) {
        case WindowMessageDirtyView:
            if (NASetContains(self->views, msg.data)) {
                ViewDisplay(msg.data);
            }
            break;
        }
    }
}

static bool WindowOnKeyEvent(KeyHandler *_self, int code)
{
    Window *self = (Window *)_self;

    if (self->nextKeyHandler) {
        return KeyHandlerHandleKeyEvent(self->nextKeyHandler, code);
    }

    return false;
}

static void WindowSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    Window *self = (Window *)_self;
    self->nextKeyHandler = next;
}
