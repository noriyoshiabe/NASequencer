#include "Controller.h"
#include "DebugWindow.h"
#include "Debug.h"
#include "KeyHandler.h"

#include <ncurses.h>

int main(int argc, char **argv)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    refresh(); 

    Controller *controller = ControllerCreate();
    DebugWindow *debugWindow = DebugWindowCreate(100, 20);

    for (;;) {
        char c = getch();
        if (c == 'q') {
            break;
        }
        KeyHandlerHandleKeyEvent(controller, c);
    }

    ControllerDestroy(controller);
    DebugWindowDestroy(debugWindow);

    endwin();
    return 0;
}
