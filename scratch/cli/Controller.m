#import "Controller.h"
#include "MessageQueue.h"

@interface Controller() {
    MessageQueue *msgQ;
}

@end

@implementation Controller

- (id)init
{
    if (self = [super init]) {
        msgQ = MessageQueueCreate();
    }
    return self;
}

- (void)setNamidi:(NAMidi *)namidi
{
    _namidi = namidi;
    [_namidi addObserver:self];
}

- (void)run:(const char *)filepath
{
    self.namidi.filepath = [NSString stringWithUTF8String:filepath];
    [self.namidi parse];

    Message msg;
    MessageQueueWait(msgQ, &msg);
#if 0
    usleep(1000 * 10);
#endif
}

- (void)namidi:(NAMidi *)namidi onParseFinish:(Sequence *)sequence
{
    [self.namidi rewind];
    [self.namidi play];
}

- (void)namidi:(NAMidi *)namidi player:(Player *)player notifyEvent:(PlayerEvent)playerEvent
{
#if 0
    if (PlayerEventReachEnd == playerEvent) {
        MessageQueuePost(msgQ, 0, NULL);
    }
#endif
}

@end
