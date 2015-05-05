#include "NAMidi.h"

int main(int argc, char **argv)
{
    NAMidi *namidi = [[NAMidi alloc] init];
    [namidi execute:argv[1]];
    return 0;
}
