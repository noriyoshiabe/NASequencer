#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "HeaderView.h"

#include <string.h>
#include <stdlib.h>

struct _HeaderView {
    InterfaceVtbl *vtbl;
    ViewNode *node;
};

static ViewNode *HeaderViewGetNode(View *self);
static void HeaderViewDraw(View *self, Size size);
static void HeaderViewDestroy(View *self);

static const ViewVtbl HeaderViewVtbl = {
    .getNode = HeaderViewGetNode,
    .draw = HeaderViewDraw,
    .destroy = HeaderViewDestroy,
};

static void *HeaderViewQueryInteface(void *self, IID iid)
{
    if (iid == IIDView)
        return (void*)&HeaderViewVtbl;
    return NULL;
}

static const InterfaceVtbl HeaderViewInterfaceVtbl = {
    .queryInterface = HeaderViewQueryInteface,
};


HeaderView *HeaderViewCreate()
{
    HeaderView *self = calloc(1, sizeof(HeaderView));
    self->vtbl = (void*)&HeaderViewInterfaceVtbl;
    self->node = ViewNodeCreate(self);
    return self;
}

static ViewNode *HeaderViewGetNode(View *_self)
{
    HeaderView *self = (HeaderView *)_self;
    return self->node;
}

static void HeaderViewDraw(View *self, Size size)
{
    ViewPrintf(self, 0, 0, "Header");
}

static void HeaderViewDestroy(View *_self)
{
    HeaderView *self = (HeaderView *)_self;
    ViewNodeDestroy(self->node);
    free(self);
}
