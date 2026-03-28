#include "WindowManager.h"
#include "Controller.h"
#include "DebugWindow.h"
#include "Debug.h"
#include "KeyHandler.h"
#include "Argument.h"
#include "NAMidi.h"
#include "Attribute.h"

#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    bool isDebug = getenv("DEBUG") != NULL;

    Argument *argument = ArgumentParse(argc, argv);
    if (argument->error) {
        puts(argument->error);
        return 1;
    }

    NAMidi *namidi = NAMidiCreate();

    MidiSourceManager *sourceManager = MidiSourceManagerSharedInstance();
    MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(sourceManager, argument->soundSource);

    MidiSourceDescription *description = MidiSourceManagerGetDefaultDescription(sourceManager);
    MidiSourceManagerSetGainForDescription(sourceManager, description, argument->gain);

    setenv("ESCDELAY", "25", 1);
    initscr();

    start_color();
    use_default_colors();

    AttributeInitializeColorPair();

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    refresh(); 

    WindowManager *windowManager = WindowManagerSharedInstance();
    Controller *controller = ControllerCreate(namidi);

    DebugWindow *debugWindow;
    if (isDebug) {
        debugWindow = DebugWindowCreate(COLS, 5);
    }

    NAMidiSetWatchEnable(namidi, true);
    NAMidiParse(namidi, argument->filepath);

    for (;;) {
        int c = getch();

        ControllerRunOnLoop(controller);

        if (c != ERR) {
            if (!WindowManagerDispatchKeyEvent(windowManager, c)) {
                if (c == 'q') {
                    break;
                }
            };
        }

        WindowManagerDisplayIfNeeded(windowManager);
        usleep(1000);
    }

    ControllerDestroy(controller);
    ArgumentDestroy(argument);

    NAMidiDestroy(namidi);

    if (isDebug) {
        DebugWindowDestroy(debugWindow);
    }

    endwin();
    return 0;
}
