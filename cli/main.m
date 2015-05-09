#import "NAMidi.h"
#import "Mixer.h"
#import "Controller.h"
#import "View.h"

#include <getopt.h>

int main(int argc, char **argv)
{
    int result = 0;
    char *outfile = NULL;
    BOOL repeat = NO;

    while (-1 != (result = getopt(argc, argv, "o:r"))) {
        switch (result) {
        case 'o':
            outfile = optarg;
            break;
        case 'r':
            repeat = YES;
            break;
        case ':':
        case '?':
            printf("usage:\n  namidi [-r] [-o <outfile>] <infile>\n");
            return 1;
        }
    }

    if (argc <= optind) {
        printf("usage:\n  namidi [-r] [-o <outfile>] <infile>\n");
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
        [[Mixer sharedInstance] initialize];
        [controller run:infile repeat:repeat];
    }

    return 0;
}
