#import "NAMidi.h"
#import "MidiSourceManager.h"
#import "Mixer.h"
#import "AudioOut.h"
#import "Controller.h"
#import "View.h"

#include <getopt.h>

int main(int argc, char **argv)
{
    int result = 0;
    char *soundfont = NULL;
    char *outfile = NULL;
    BOOL repeat = NO;

    while (-1 != (result = getopt(argc, argv, "o:s:r"))) {
        switch (result) {
        case 'o':
            outfile = optarg;
            break;
        case 'r':
            repeat = YES;
            break;
        case 's':
            soundfont = optarg;
            break;
        case ':':
        case '?':
            printf("usage:\n  namidi [-r] [-s <soundfont>] [-o <outfile>] <infile>\n");
            return 1;
        }
    }

    if (argc <= optind) {
        printf("usage:\n  namidi [-r] [-s <soundfont>] [-o <outfile>] <infile>\n");
        return 1;
    }

    const char *infile = argv[optind];

    NAMidi *namidi = [[NAMidi alloc] init];
    Controller *controller = [[Controller alloc] init];
    controller.namidi = namidi;
    View *view = [[View alloc] init];
    view.namidi = namidi;

    if (outfile) {
        [controller write:infile outfile:outfile];
    } 
    else {
        MidiSourceManager *manager = [MidiSourceManager sharedInstance];
        [manager loadSoundFont:[NSString stringWithUTF8String:soundfont]];
        
        Mixer *mixer = [[Mixer alloc] init];
        namidi.player.mixer = mixer;

        [controller run:infile repeat:repeat];
    }

    return 0;
}
