#include "ErrorWindow.h"
#include "WindowManager.h"
#include "Interface.h"
#include "Window.h"
#include "KeyHandler.h"
#include "ParseInfo.h"

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

struct _ErrorWindow {
    const InterfaceVtbl *vtbl;
    Window *window;
    Rect frame;
    KeyHandler *nextKeyHandler;
};

static bool ErrorWindowOnKeyEvent(KeyHandler *self, int code);
static void ErrorWindowSetNextKeyHandler(KeyHandler *self, KeyHandler *keyHandler);
static KeyHandler *ErrorWindowGetNextKeyHandler(KeyHandler *self);

static const KeyHandlerVtbl ErrorWindowKeyHandlerVtbl = {
    .onKeyEvent = ErrorWindowOnKeyEvent,
    .setNextKeyHandler = ErrorWindowSetNextKeyHandler,
    .getNextKeyHandler = ErrorWindowGetNextKeyHandler,
};

static void *ErrorWindowQueryInteface(void *self, IID iid)
{
    if (iid == IIDKeyHandler)
        return (void*)&ErrorWindowKeyHandlerVtbl;
    return NULL;
}

static const InterfaceVtbl ErrorWindowInterfaceVtbl = {
    .queryInterface = ErrorWindowQueryInteface,
};

ErrorWindow *ErrorWindowCreate()
{
    ErrorWindow *self = calloc(1, sizeof(ErrorWindow));
    self->vtbl = &ErrorWindowInterfaceVtbl;

    self->frame = (Rect){{2, 2}, {COLS - 4, LINES - 4}};
    self->window = WindowCreate(self->frame);
    WindowSetKeyView(self->window, self);

    return self;
}

void ErrorWindowDraw(ErrorWindow *self, NAArray *errors)
{
    char line[self->frame.size.width + 1];
    memset(line, ' ', self->frame.size.width);
    line[self->frame.size.width] = '\0';

    for (int i = 0; i < self->frame.size.height; ++i) {
        WindowPrint(self->window, 0, i, line);
    }

    WindowSetBox(self->window);
    WindowSetAttr(self->window, A_NORMAL);

    int count = NAArrayCount(errors);
    for (int i = 0; i < count; ++i) {
        char *message = ParseErrorFormattedString(NAArrayGetValueAt(errors, i));
        int length = strlen(message);
        if (self->frame.size.width - 4 < length) {
            message[self->frame.size.width - 4] = '\0';
        }
        WindowPrint(self->window, 2, i + 1, message);
        free(message);
    }

    WindowRefresh(self->window);
}

void ErrorWindowDestroy(ErrorWindow *self)
{
    WindowDestroy(self->window);
    free(self);
}

static bool ErrorWindowOnKeyEvent(KeyHandler *_self, int code)
{
    ErrorWindow *self = _self;

    if (code == 27) {
        ErrorWindowDestroy(self);
        return true;
    }

    return false;
}

static void ErrorWindowSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    ErrorWindow *self = _self;
    self->nextKeyHandler = next;
}

static KeyHandler *ErrorWindowGetNextKeyHandler(KeyHandler *_self)
{
    ErrorWindow *self = _self;
    return self->nextKeyHandler;
}
