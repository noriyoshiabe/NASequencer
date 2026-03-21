#include "Window.h"
#include "Debug.h"

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

struct _Window {
    const InterfaceVtbl *vtbl;
    WINDOW *window;
    KeyHandler *nextKeyHandler;
};

static bool WindowOnKeyEvent(KeyHandler *self, char code);
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
    self->window = newwin(rect.size.height, rect.size.width, rect.point.y, rect.point.x);
    return self;
}

void WindowDestroy(Window *self)
{
    delwin(self->window);
    free(self);
}

void WindowPrint(Window *self, int x, int y, const char *str)
{
    mvwprintw(self->window, y, x, str);
}

void WindowRefresh(Window *self)
{
    wrefresh(self->window);
}

static bool WindowOnKeyEvent(KeyHandler *_self, char code)
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
