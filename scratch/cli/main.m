#import "NAMidi.h"
#import "Controller.h"
#import "View.h"

int main(int argc, char **argv)
{
    NAMidi *namidi = [[NAMidi alloc] init];
    Controller *controller = [[Controller alloc] init];
    controller.namidi = namidi;
    View *view = [[View alloc] init];
    view.namidi = namidi;

    [controller run: 1 < argc ? argv[1] : NULL];
    return 0;
}
