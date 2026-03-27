#include "SynthesizerWindow.h"
#include "Interface.h"
#include "Window.h"
#include "KeyHandler.h"
#include "Debug.h"

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

struct _SynthesizerWindow {
    const InterfaceVtbl *vtbl;
    Window *window;
    Rect frame;
    KeyHandler *nextKeyHandler;
    int channel;
    Controller *controller;
};

static bool SynthesizerWindowOnKeyEvent(KeyHandler *self, int code);
static void SynthesizerWindowSetNextKeyHandler(KeyHandler *self, KeyHandler *keyHandler);
static KeyHandler *SynthesizerWindowGetNextKeyHandler(KeyHandler *self);

static const KeyHandlerVtbl SynthesizerWindowKeyHandlerVtbl = {
    .onKeyEvent = SynthesizerWindowOnKeyEvent,
    .setNextKeyHandler = SynthesizerWindowSetNextKeyHandler,
    .getNextKeyHandler = SynthesizerWindowGetNextKeyHandler,
};

static void *SynthesizerWindowQueryInteface(void *self, IID iid)
{
    if (iid == IIDKeyHandler)
        return (void*)&SynthesizerWindowKeyHandlerVtbl;
    return NULL;
}

static const InterfaceVtbl SynthesizerWindowInterfaceVtbl = {
    .queryInterface = SynthesizerWindowQueryInteface,
};

SynthesizerWindow *SynthesizerWindowCreate(int channel)
{
    SynthesizerWindow *self = calloc(1, sizeof(SynthesizerWindow));
    self->vtbl = &SynthesizerWindowInterfaceVtbl;

    self->frame = (Rect){{(COLS - 69) / 2, (LINES - 13) / 2}, {69, 13}};
    self->window = WindowCreate(self->frame);
    WindowSetKeyView(self->window, self);

    self->channel = channel;

    WindowSetBox(self->window);

    return self;
}

void SynthesizerWindowDestroy(SynthesizerWindow *self)
{
    WindowDestroy(self->window);
    free(self);
}

int SynthesizerWindowGetChannel(SynthesizerWindow *self)
{
    return self->channel;
}

void SynthesizerWindowSetController(SynthesizerWindow *self, Controller *controller)
{
    self->controller = controller;
}

static bool SynthesizerWindowOnKeyEvent(KeyHandler *_self, int code)
{
    SynthesizerWindow *self = _self;

    __Dump__C(code);

    if (code == 27) {
        ControllerPostMessage(self->controller, ControllerMessageCloseSynthesizerWindow, NULL);
        return true;
    }

    return false;
}

static void SynthesizerWindowSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    SynthesizerWindow *self = _self;
    self->nextKeyHandler = next;
}

static KeyHandler *SynthesizerWindowGetNextKeyHandler(KeyHandler *_self)
{
    SynthesizerWindow *self = _self;
    return self->nextKeyHandler;
}
