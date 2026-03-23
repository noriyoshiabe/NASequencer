#include "Controller.h"
#include "View.h"
#include "KeyHandler.h"
#include "MainView.h"
#include "Debug.h"

#include <stdlib.h>
#include <ncurses.h>

struct _Controller {
    const InterfaceVtbl *vtbl;
    MainView *mainView;
    NAMidi *namidi;
};

static bool ControllerOnKeyEvent(KeyHandler *self, int code);
static void ControllerSetNextKeyHandler(KeyHandler *self, KeyHandler *keyHandler);
static KeyHandler *ControllerGetNextKeyHandler(KeyHandler *self);

static const KeyHandlerVtbl ControllerKeyHandlerVtbl = {
    .onKeyEvent = ControllerOnKeyEvent,
    .setNextKeyHandler = ControllerSetNextKeyHandler,
    .getNextKeyHandler = ControllerGetNextKeyHandler,
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

    self->mainView = MainViewCreate(namidi);
    KeyHandlerSetNextKeyHandler(self->mainView, self);

    self->namidi = namidi;
    NAMidiAddObserver(self->namidi, self, &ControllerNAMidiObserverCallbacks);

    return self;
}

void ControllerDestroy(Controller *self)
{
    NAMidiRemoveObserver(self->namidi, self);

    ViewDestroy((View *)self->mainView);
    free(self);
}

static bool ControllerOnKeyEvent(KeyHandler *_self, int code)
{
    Controller *self = _self;
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

    return false;
}

static void ControllerSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
}

static KeyHandler *ControllerGetNextKeyHandler(KeyHandler *_self)
{
    return NULL;
}

static void ControllerNAMidiOnBeforeParse(void *receiver, bool fileChanged)
{
}

static void ControllerNAMidiOnParseFinish(void *receiver, Sequence *sequence, ParseInfo *info)
{
    //Controller *self = receiver;

    NAIterator *iterator = NAArrayGetIterator(info->errors);
    while (iterator->hasNext(iterator)) {
        ParseError *error = iterator->next(iterator);
        char *formatted = ParseErrorFormattedString(error);
        free(formatted);
        // TODO error display
    }
}

static NAMidiObserverCallbacks ControllerNAMidiObserverCallbacks = {
    ControllerNAMidiOnBeforeParse,
    ControllerNAMidiOnParseFinish,
};
