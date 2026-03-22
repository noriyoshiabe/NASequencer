#include "DebugWindow.h"
#include "Debug.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define BUFFER_LENGTH 256

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
        self->buffers[i] = calloc(BUFFER_LENGTH, sizeof(char));
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
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    struct tm *now = localtime(&ts.tv_sec);

    char ymdhms[21];
    strftime(ymdhms, sizeof(ymdhms), "%Y-%m-%d %H:%M:%S", now);
    char timestamp[25];
    sprintf(timestamp, "%s.%03ld ", ymdhms, ts.tv_nsec / 1000000);

    if (self->height <= self->logCount) {
        for (int i = 0; i < self->height - 1; ++i) {
            memcpy(self->buffers[i], self->buffers[i + 1], BUFFER_LENGTH);
        }
    }

    int index = MIN(self->logCount, self->height - 1);
    sprintf(self->buffers[index], "%s", timestamp);
    vsnprintf(self->buffers[index] + strlen(timestamp), BUFFER_LENGTH - strlen(timestamp), fmt, argList);

    ++self->logCount;

    for (int i = 0; i < self->height; ++i) {
        mvwprintw(self->window, i, 0, self->buffers[i]);
    }

    wrefresh(self->window);
}
