#include "Controller.h"
#include "Debug.h"
#include "DebugWindow.h"

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
        if (c == 'q')
            break;
        Debug("####### %c", c);
    }

    ControllerDestroy(controller);
    DebugWindowDestroy(debugWindow);

    endwin();
    return 0;
}
