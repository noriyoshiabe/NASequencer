#include "Controller.h"
#include "View.h"
#include "Window.h"
#include "MainView.h"
#include "Debug.h"

#include <ncurses.h>
#include <stdlib.h>

struct _Controller {
    Window *mainWindow;
    MainView *mainView;
};

Controller *ControllerCreate()
{
    Controller *self = calloc(1, sizeof(Controller));

    Rect frame = {{0, 0}, {COLS, LINES}};
    self->mainWindow = WindowCreate(frame);
    self->mainView = MainViewCreate();

    ViewSetFrame(self->mainView, frame);
    ViewSetWindow(self->mainView, self->mainWindow);
    ViewInvalidate(self->mainView);

    return self;
}

void ControllerDestroy(Controller *self)
{
    ViewDestroy((View *)self->mainView);
    WindowDestroy(self->mainWindow);
    free(self);
}
