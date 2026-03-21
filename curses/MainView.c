#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "MainView.h"
#include "HeaderView.h"

#include <string.h>
#include <stdlib.h>

struct _MainView {
    InterfaceVtbl *vtbl;
    ViewNode *node;
};

static ViewNode *MainViewGetNode(View *self);
static void MainViewDraw(View *self, Size size);
static void MainViewDestroy(View *self);

static const ViewVtbl MainViewVtbl = {
    .getNode = MainViewGetNode,
    .draw = MainViewDraw,
    .destroy = MainViewDestroy,
};

static void *MainViewQueryInteface(void *self, IID iid)
{
    if (iid == IIDView)
        return (void*)&MainViewVtbl;
    return NULL;
}

static const InterfaceVtbl MainViewInterfaceVtbl = {
    .queryInterface = MainViewQueryInteface,
};


MainView *MainViewCreate()
{
    MainView *self = calloc(1, sizeof(MainView));
    self->vtbl = (void*)&MainViewInterfaceVtbl;
    self->node = ViewNodeCreate(self);

    HeaderView *header = HeaderViewCreate();
    Rect rect = {{1, 1}, {100, 2}};
    ViewSetFrame(header, rect);
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
