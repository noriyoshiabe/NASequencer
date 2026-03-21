#include "Controller.h"
#include "View.h"
#include "Window.h"
#include "MainView.h"
#include "Debug.h"

#include <ncurses.h>
#include <stdlib.h>

struct _Controller {
    const InterfaceVtbl *vtbl;
    Window *mainWindow;
    MainView *mainView;
    KeyHandler *nextKeyHandler;
};

static bool ControllerOnKeyEvent(KeyHandler *self, char code);
static void ControllerSetNextKeyHandler(KeyHandler *self, KeyHandler *keyHandler);

static const KeyHandlerVtbl ControllerKeyHandlerVtbl = {
    .onKeyEvent = ControllerOnKeyEvent,
    .setNextKeyHandler = ControllerSetNextKeyHandler,
};

static void *ControllerQueryInteface(void *self, IID iid)
{
    if (iid == IIDKeyHandler)
        return (void*)&ControllerKeyHandlerVtbl;
    return NULL;
}

static const InterfaceVtbl ControllerInterfaceVtbl = {
    .queryInterface = ControllerQueryInteface,
};

Controller *ControllerCreate()
{
    Controller *self = calloc(1, sizeof(Controller));
    self->vtbl = &ControllerInterfaceVtbl;

    Rect frame = {{0, 0}, {COLS, LINES}};
    self->mainWindow = WindowCreate(frame);
    self->mainView = MainViewCreate();

    ViewSetFrame(self->mainView, frame);
    ViewSetWindow(self->mainView, self->mainWindow);
    ViewInvalidate(self->mainView);

    KeyHandlerSetNextKeyHandler(self, self->mainWindow);
    KeyHandlerSetNextKeyHandler(self->mainWindow, self->mainView);

    return self;
}

void ControllerDestroy(Controller *self)
{
    ViewDestroy((View *)self->mainView);
    WindowDestroy(self->mainWindow);
    free(self);
}

static bool ControllerOnKeyEvent(KeyHandler *_self, char code)
{
    Controller *self = (Controller *)_self;

    if (self->nextKeyHandler) {
        return KeyHandlerHandleKeyEvent(self->nextKeyHandler, code);
    }

    return false;
}

static void ControllerSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    Controller *self = (Controller *)_self;
    self->nextKeyHandler = next;
}
