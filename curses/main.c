#include "WindowManager.h"
#include "Controller.h"
#include "DebugWindow.h"
#include "Debug.h"
#include "KeyHandler.h"
#include "Argument.h"
#include "NAMidi.h"
#include "Color.h"

#include <ncurses.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    Argument *argument = ArgumentParse(argc, argv);
    if (argument->error) {
        puts(argument->error);
        return 1;
    }

    NAMidi *namidi = NAMidiCreate();

    MidiSourceManager *sourceManager = MidiSourceManagerSharedInstance();
    for (char **source = argument->soundSources; NULL != *source; ++source) {
        MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(sourceManager, *source);
    }

    NAArray *descriptions = MidiSourceManagerGetAvailableDescriptions(sourceManager);
    NAIterator *iterator = NAArrayGetIterator(descriptions);
    while (iterator->hasNext(iterator)) {
        MidiSourceDescription *description = iterator->next(iterator);
        MidiSourceManagerSetGainForDescription(sourceManager, description, argument->gain);
    }

    initscr();

    start_color();
    use_default_colors();
    init_pair(ColorFocused, COLOR_WHITE, COLOR_BLUE);
    init_pair(ColorSelected, COLOR_WHITE, COLOR_RED);
    init_pair(ColorMute, COLOR_BLUE, COLOR_YELLOW);
    init_pair(ColorSolo, COLOR_BLUE, COLOR_GREEN);
    init_pair(ColorLevelLow, COLOR_GREEN, -1);
    init_pair(ColorLevelHigh, COLOR_RED, -1);

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    refresh(); 

    WindowManager *windowManager = WindowManagerSharedInstance();
    Controller *controller = ControllerCreate(namidi);
    DebugWindow *debugWindow = DebugWindowCreate(COLS, 5);

    NAMidiSetWatchEnable(namidi, true);
    NAMidiParse(namidi, argument->filepath);

    for (;;) {
        int c = getch();
        if (c == 'q') {
            break;
        }

        if (c != ERR) {
            WindowManagerDispatchKeyEvent(windowManager, c);
        }

        WindowManagerDisplayIfNeeded(windowManager);
        usleep(100);
    }

    ControllerDestroy(controller);
    DebugWindowDestroy(debugWindow);
    ArgumentDestroy(argument);

    NAMidiDestroy(namidi);

    endwin();
    return 0;
}
