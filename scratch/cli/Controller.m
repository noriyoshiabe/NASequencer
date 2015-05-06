#import "Controller.h"

@implementation Controller

- (void)setNamidi:(NAMidi *)namidi
{
    _namidi = namidi;
    [_namidi addObserver:self];
}

- (void)run:(const char *)filepath
{
    [self.namidi parse:[NSString stringWithUTF8String:filepath]];
}

@end
