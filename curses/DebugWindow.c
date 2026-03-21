#include "DebugWindow.h"
#include "Debug.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

struct _DebugWindow {
    WINDOW *window;
    char **buffers;
    int width;
    int height;
    int logCount;
};

DebugWindow *DebugWindowCreate(int width, int height)
{
    DebugWindow *self = calloc(1, sizeof(DebugWindow));
    self->window = newwin(height, width, LINES - height, COLS - width);
    self->height = height;
    self->buffers = calloc(self->height, sizeof(char *));

    for (int i = 0; i < self->height; ++i) {
        self->buffers[i] = calloc(256, sizeof(char));
    }

    DebugInit(self);

    return self;
}

void DebugWindowDestroy(DebugWindow *self)
{
    delwin(self->window);

    for (int i = 0; i < self->height; ++i) {
        free(self->buffers[i]);
    }
    free(self->buffers);
    free(self);
}

void DebugWindowAppendLog(DebugWindow *self, const char *fmt, va_list argList)
{
    if (self->height <= self->logCount) {
        for (int i = 0; i < self->height - 1; ++i) {
            memcpy(self->buffers[i], self->buffers[i + 1], 256);
        }
    }

    int index = MIN(self->logCount, self->height - 1);
    vsnprintf(self->buffers[index], 256, fmt, argList);

    ++self->logCount;

    for (int i = 0; i < self->height; ++i) {
        mvwprintw(self->window, i, 0, self->buffers[i]);
    }

    wrefresh(self->window);
}
