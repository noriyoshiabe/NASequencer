#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "MainView.h"
#include "HeaderView.h"
#include "ChannelView.h"
#include "KeyHandler.h"
#include "Debug.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

struct _MainView {
    const InterfaceVtbl *vtbl;
    ViewNode *node;
    KeyHandler *nextKeyHandler;
    NAMidi *namidi;
    NAArray *channelViews;
    bool channelExsits[16];
};

static ViewNode *MainViewGetNode(View *self);
static void MainViewDraw(View *self, Size size);
static void MainViewDestroy(View *self);

static const ViewVtbl MainViewViewVtbl = {
    .getNode = MainViewGetNode,
    .draw = MainViewDraw,
    .destroy = MainViewDestroy,
};

static bool MainViewOnKeyEvent(KeyHandler *self, int code);
static void MainViewSetNextKeyHandler(KeyHandler *self, KeyHandler *next);

static const KeyHandlerVtbl MainViewKeyHandlerVtbl = {
    .onKeyEvent = MainViewOnKeyEvent,
    .setNextKeyHandler = MainViewSetNextKeyHandler,
};

static void *MainViewQueryInteface(void *self, IID iid)
{
    if (iid == IIDView)
        return (void*)&MainViewViewVtbl;
    if (iid == IIDKeyHandler)
        return (void*)&MainViewKeyHandlerVtbl;
    return NULL;
}

static const InterfaceVtbl MainViewInterfaceVtbl = {
    .queryInterface = MainViewQueryInteface,
};

static NAMidiObserverCallbacks MainViewNAMidiObserverCallbacks;

MainView *MainViewCreate(NAMidi *namidi)
{
    MainView *self = calloc(1, sizeof(MainView));
    self->vtbl = &MainViewInterfaceVtbl;
    self->node = ViewNodeCreate(self);

    HeaderView *header = HeaderViewCreate(namidi);
    Rect frame = {{0, 0}, {COLS, 3}};
    ViewSetFrame(header, frame);
    ViewAppendChild(self, header);

    self->channelViews = NAArrayCreate(16, NULL);
    for (int i = 0; i < 16; ++i) {
        ChannelView *channelView = ChannelViewCreate(NAMidiGetMixer(namidi), i + 1);
        ViewAppendChild(self, channelView);
        NAArrayAppend(self->channelViews, channelView);
    }

    self->namidi = namidi;
    NAMidiAddObserver(self->namidi, self, &MainViewNAMidiObserverCallbacks);

    return self;
}

static ViewNode *MainViewGetNode(View *_self)
{
    MainView *self = (MainView *)_self;
    return self->node;
}

static void MainViewDraw(View *_self, Size size)
{
    MainView *self = (MainView *)_self;

    int offsetY = 3;

    for (int i = 0; i < 16; ++i) {
        Rect frame = {{0, offsetY}, {COLS, 5}};
        ChannelView *channelView = NAArrayGetValueAt(self->channelViews, i);
        
        if (self->channelExsits[i]) {
            ViewSetFrame(channelView, frame);
            ViewSetHidden(channelView, false);
            offsetY += 5;
        } else {
            ViewSetHidden(channelView, true);
        }
    }
}

static void MainViewDestroy(View *_self)
{
    MainView *self = (MainView *)_self;

    NAMidiRemoveObserver(self->namidi, self);
    NAArrayDestroy(self->channelViews);
    ViewNodeDestroy(self->node);
    free(self);
}

static bool MainViewOnKeyEvent(KeyHandler *_self, int code)
{
    MainView *self = (MainView *)_self;

    if (self->nextKeyHandler) {
        return KeyHandlerHandleKeyEvent(self->nextKeyHandler, code);
    }

    __Dump__P(self);

    return false;
}

static void MainViewSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    MainView *self = (MainView *)_self;
    self->nextKeyHandler = next;
}

static void MainViewNAMidiOnBeforeParse(void *receiver, bool fileChanged)
{
}

static void MainViewNAMidiOnParseFinish(void *receiver, Sequence *sequence, ParseInfo *info)
{
    MainView *self = receiver;

    for (int i = 0; i < 16; ++i) {
        self->channelExsits[i] = false;
    }

    NAIterator *iterator = NAArrayGetIterator(sequence->events);
    while (iterator->hasNext(iterator)) {
        MidiEvent *event = iterator->next(iterator);
        switch (event->type) {
        case MidiEventTypeTempo:
        case MidiEventTypeTime:
        case MidiEventTypeKey:
        case MidiEventTypeMarker:
            break;
        default:
            self->channelExsits[((ChannelEvent *)event)->channel - 1] = true;
            break;
        }
    }

    ViewInvalidate(self);
}

static NAMidiObserverCallbacks MainViewNAMidiObserverCallbacks = {
    MainViewNAMidiOnBeforeParse,
    MainViewNAMidiOnParseFinish,
};
