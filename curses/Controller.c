#include "Controller.h"
#include "View.h"
#include "KeyHandler.h"
#include "MainView.h"
#include "Debug.h"
#include "ErrorWindow.h"
#include "SynthesizerWindow.h"
#include "NAMessageQ.h"

#include <stdlib.h>
#include <ncurses.h>

struct _Controller {
    const InterfaceVtbl *vtbl;
    MainView *mainView;
    NAMidi *namidi;
    NAMessageQ *msgQ;
    ErrorWindow *errorWindow;
    SynthesizerWindow *synthesizerWindow;
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
    MainViewSetController(self->mainView, self);

    self->namidi = namidi;
    NAMidiAddObserver(self->namidi, self, &ControllerNAMidiObserverCallbacks);

    self->msgQ = NAMessageQCreate(4);

    return self;
}

void ControllerDestroy(Controller *self)
{
    NAMidiRemoveObserver(self->namidi, self);

    ViewDestroy((View *)self->mainView);
    free(self);
}

void ControllerPostMessage(Controller *self, int kind, void *data)
{
    NAMessageQPost(self->msgQ, kind, data);
}

void ControllerProcessMessage(Controller *self)
{
    NAMessage msg;

    while (NAMessageQPeek(self->msgQ, &msg)) {
        switch (msg.kind) {
        case ControllerMessageParseErrors:
            {
                NAArray *errors = msg.data;

                if (0 < NAArrayCount(errors)) {
                    if (!self->errorWindow) {
                        self->errorWindow = ErrorWindowCreate();
                        KeyHandlerSetNextKeyHandler(self->errorWindow, self);
                    }

                    ErrorWindowDraw(self->errorWindow, errors);
                }
                else {
                    if (self->errorWindow) {
                        ErrorWindowDestroy(self->errorWindow);
                        self->errorWindow = NULL;
                    }
                }
            }
            break;
        case ControllerMessageOpenSynthesizerWindow:
            {
                int channel = *((int *)msg.data);
                self->synthesizerWindow = SynthesizerWindowCreate(channel);
                KeyHandlerSetNextKeyHandler(self->synthesizerWindow, self);
                SynthesizerWindowSetController(self->synthesizerWindow, self);
            }
            break;
        case ControllerMessageCloseSynthesizerWindow:
            SynthesizerWindowDestroy(self->synthesizerWindow);
            self->synthesizerWindow = NULL;
            break;
        }
    }
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
    case ',':
        PlayerBackWard(player);
        return true;
    case '.':
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
    Controller *self = receiver;
    NAMessageQPost(self->msgQ, ControllerMessageParseErrors, info->errors);

    if (self->synthesizerWindow) {
        int channel = SynthesizerWindowGetChannel(self->synthesizerWindow);
        if (!MainViewGetChannelExists(self->mainView, channel)) {
            NAMessageQPost(self->msgQ, ControllerMessageCloseSynthesizerWindow, NULL);
        }
    }
}

static NAMidiObserverCallbacks ControllerNAMidiObserverCallbacks = {
    ControllerNAMidiOnBeforeParse,
    ControllerNAMidiOnParseFinish,
};
