#include "Window.h"

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

struct _Window {
    WINDOW *window;
};

Window *WindowCreate(Rect rect)
{
    Window *self = calloc(1, sizeof(Window));
    self->window = newwin(rect.size.height, rect.size.width, rect.point.y, rect.point.x);
    return self;
}

void WindowDestroy(Window *self)
{
    delwin(self->window);
    free(self);
}

void WindowPrint(Window *self, int x, int y, const char *str)
{
    mvwprintw(self->window, y, x, str);
}

void WindowRefresh(Window *self)
{
    wrefresh(self->window);
}
