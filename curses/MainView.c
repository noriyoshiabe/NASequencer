#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "MainView.h"
#include "HeaderView.h"
#include "KeyHandler.h"
#include "Debug.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

struct _MainView {
    const InterfaceVtbl *vtbl;
    ViewNode *node;
    KeyHandler *nextKeyHandler;
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

MainView *MainViewCreate(NAMidi *namidi)
{
    MainView *self = calloc(1, sizeof(MainView));
    self->vtbl = &MainViewInterfaceVtbl;
    self->node = ViewNodeCreate(self);

    HeaderView *header = HeaderViewCreate(namidi);
    Rect frame = {{0, 0}, {COLS, 3}};
    ViewSetFrame(header, frame);
    ViewAppendChild(self, header);

    return self;
}

static ViewNode *MainViewGetNode(View *_self)
{
    MainView *self = (MainView *)_self;
    return self->node;
}

static void MainViewDraw(View *self, Size size)
{
    ViewPrintf(self, 2, 4, "Main");
}

static void MainViewDestroy(View *_self)
{
    MainView *self = (MainView *)_self;
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
