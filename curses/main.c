#include "Controller.h"
#include "DebugWindow.h"
#include "Debug.h"
#include "KeyHandler.h"
#include "Argument.h"
#include "NAMidi.h"

#include <ncurses.h>

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
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    refresh(); 

    Controller *controller = ControllerCreate(namidi);
    DebugWindow *debugWindow = DebugWindowCreate(COLS, 5);

    NAMidiSetWatchEnable(namidi, true);
    NAMidiParse(namidi, argument->filepath);

    for (;;) {
        int c = getch();
        if (c == 'q') {
            break;
        }
        KeyHandlerHandleKeyEvent(controller, c);
    }

    ControllerDestroy(controller);
    DebugWindowDestroy(debugWindow);
    ArgumentDestroy(argument);

    NAMidiDestroy(namidi);

    endwin();
    return 0;
}
