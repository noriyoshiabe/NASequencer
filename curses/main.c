#include <ncurses.h>

#include "View.h"
#include "Window.h"
#include "MainView.h"

int main(int argc, char **argv)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    refresh(); 

    Rect frame = {
        .point = { .x = 0, .y = 0 },
        .size = { .width = COLS, .height = LINES},
    };

    Window *window = WindowCreate(frame);

    MainView *mainView = MainViewCreate();

    ViewSetWindow(mainView, window);

    ViewSetFrame(mainView, frame);
    ViewInvalidate(mainView);

    getch();

    WindowDestroy(window);
    ViewDestroy((View *)mainView);

    endwin();
    
    return 0;
}
