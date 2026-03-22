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
    NAMidi *namidi;
};

static bool ControllerOnKeyEvent(KeyHandler *self, int code);
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

static NAMidiObserverCallbacks ControllerNAMidiObserverCallbacks;

Controller *ControllerCreate(NAMidi *namidi)
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

    self->namidi = namidi;
    NAMidiAddObserver(self->namidi, self, &ControllerNAMidiObserverCallbacks);

    return self;
}

void ControllerDestroy(Controller *self)
{
    NAMidiRemoveObserver(self->namidi, self);

    ViewDestroy((View *)self->mainView);
    WindowDestroy(self->mainWindow);
    free(self);
}

static bool ControllerOnKeyEvent(KeyHandler *_self, int code)
{
    Controller *self = (Controller *)_self;
    Player *player = NAMidiGetPlayer(self->namidi);

    __Dump__C(code);

    switch (code) {
    case ' ':
        PlayerPlayPause(player);
        return true;
    case 'r':
        PlayerRewind(player);
        return true;
    case KEY_LEFT:
        PlayerBackWard(player);
        return true;
    case KEY_RIGHT:
        PlayerForward(player);
        return true;
    case '<':
        PlayerBackWardToMarker(player);
        return true;
    case '>':
        PlayerForwardToMarker(player);
        return true;
    }

    if (self->nextKeyHandler) {
        return KeyHandlerHandleKeyEvent(self->nextKeyHandler, code);
    }

    return false;
}

static void ControllerNAMidiOnBeforeParse(void *receiver, bool fileChanged)
{
}

static void ControllerNAMidiOnParseFinish(void *receiver, Sequence *sequence, ParseInfo *info)
{
    Controller *self = receiver;

    // TODO process sequence

    ViewInvalidate(self->mainView);

    __Trace__;

    NAIterator *iterator = NAArrayGetIterator(info->errors);
    while (iterator->hasNext(iterator)) {
        ParseError *error = iterator->next(iterator);
        char *formatted = ParseErrorFormattedString(error);
        free(formatted);
        // TODO error display
    }
}

static void ControllerSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    Controller *self = (Controller *)_self;
    self->nextKeyHandler = next;
}

static NAMidiObserverCallbacks ControllerNAMidiObserverCallbacks = {
    ControllerNAMidiOnBeforeParse,
    ControllerNAMidiOnParseFinish,
};
