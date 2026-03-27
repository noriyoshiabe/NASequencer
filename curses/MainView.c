#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "MainView.h"
#include "HeaderView.h"
#include "ChannelView.h"
#include "MasterView.h"
#include "KeyHandler.h"
#include "Debug.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

struct _MainView {
    const InterfaceVtbl *vtbl;
    ViewNode *node;
    KeyHandler *nextKeyHandler;
    Window *window;
    NAMidi *namidi;
    NAArray *channelViews;
    NAArray *activeChannelViews;
    MasterView *masterView;
    int focusedViewIndex;
    bool channelExsits[16];
    Controller *controller;
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
static KeyHandler *MainViewGetNextKeyHandler(KeyHandler *self);

static const KeyHandlerVtbl MainViewKeyHandlerVtbl = {
    .onKeyEvent = MainViewOnKeyEvent,
    .setNextKeyHandler = MainViewSetNextKeyHandler,
    .getNextKeyHandler = MainViewGetNextKeyHandler,
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

    Rect frame = {{0, 0}, {COLS, LINES}};
    self->window = WindowCreate(frame);

    ViewSetWindow(self, self->window);
    ViewSetFrame(self, frame);

    HeaderView *header = HeaderViewCreate(namidi);
    frame = (Rect){{0, 0}, {COLS, 3}};
    ViewSetFrame(header, frame);
    ViewAppendChild(self, header);

    self->channelViews = NAArrayCreate(16, NULL);
    self->activeChannelViews = NAArrayCreate(16, NULL);

    for (int i = 0; i < 16; ++i) {
        ChannelView *channelView = ChannelViewCreate(NAMidiGetMixer(namidi), i + 1);
        ViewAppendChild(self, channelView);
        KeyHandlerSetNextKeyHandler(channelView, self);
        NAArrayAppend(self->channelViews, channelView);
    }

    self->masterView = MasterViewCreate(NAMidiGetMixer(namidi));
    ViewAppendChild(self, self->masterView);
    KeyHandlerSetNextKeyHandler(self->masterView, self);

    self->namidi = namidi;
    NAMidiAddObserver(self->namidi, self, &MainViewNAMidiObserverCallbacks);

    return self;
}

bool MainViewGetChannelExists(MainView *self, int channel)
{
    return self->channelExsits[channel - 1];
}

void MainViewSetController(MainView *self, Controller *controller)
{
    self->controller = controller;

    NAIterator *iterator = NAArrayGetIterator(self->channelViews);
    while (iterator->hasNext(iterator)) {
        ChannelView *channelView = iterator->next(iterator);
        ChannelViewSetController(channelView, self->controller);
    }
}

static ViewNode *MainViewGetNode(View *_self)
{
    MainView *self = (MainView *)_self;
    return self->node;
}

static void MainViewDraw(View *_self, Size size)
{
    MainView *self = _self;

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

    Rect frame = {{0, offsetY}, {COLS, 3}};
    ViewSetFrame(self->masterView, frame);
}

static void MainViewDestroy(View *_self)
{
    MainView *self = (MainView *)_self;

    NAMidiRemoveObserver(self->namidi, self);

    NAArrayDestroy(self->channelViews);
    NAArrayDestroy(self->activeChannelViews);
    ViewNodeDestroy(self->node);
    WindowDestroy(self->window);
    free(self);
}

static bool MainViewOnKeyEvent(KeyHandler *_self, int code)
{
    MainView *self = _self;

    int activeViewCount = NAArrayCount(self->activeChannelViews) + 1;

    switch (code) {
    case KEY_DOWN:
        ++self->focusedViewIndex;

        if (activeViewCount <= self->focusedViewIndex) {
            self->focusedViewIndex = activeViewCount - 1;
        }

        if (activeViewCount - 1 <= self->focusedViewIndex) {
            ViewBecomeKeyView(self->masterView);
        } else {
            ViewBecomeKeyView(NAArrayGetValueAt(self->activeChannelViews, self->focusedViewIndex));
        }
        return true;
    case KEY_UP:
        --self->focusedViewIndex;

        if (self->focusedViewIndex < 0) {
            self->focusedViewIndex = 0;
        }

        ViewBecomeKeyView(NAArrayGetValueAt(self->activeChannelViews, self->focusedViewIndex));
        return true;
    }

    __Dump__P(self);

    return false;
}

static void MainViewSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    MainView *self = _self;
    self->nextKeyHandler = next;
}

static KeyHandler *MainViewGetNextKeyHandler(KeyHandler *_self)
{
    MainView *self = _self;
    return self->nextKeyHandler;
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

    NAArrayRemoveAll(self->activeChannelViews);

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

    for (int i = 0; i < 16; ++i) {
        if (self->channelExsits[i]) {
            NAArrayAppend(self->activeChannelViews, NAArrayGetValueAt(self->channelViews, i));
        }
    }

    int activeChannelViewCount = NAArrayCount(self->activeChannelViews);
    if (activeChannelViewCount <= self->focusedViewIndex) {
        self->focusedViewIndex = activeChannelViewCount - 1;
    }

    if (0 < activeChannelViewCount) {
        ViewBecomeKeyView(NAArrayGetValueAt(self->activeChannelViews, self->focusedViewIndex));
    }

    ViewInvalidate(self);
}

static NAMidiObserverCallbacks MainViewNAMidiObserverCallbacks = {
    MainViewNAMidiOnBeforeParse,
    MainViewNAMidiOnParseFinish,
};
