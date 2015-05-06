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
    [self.namidi parse:[NSString stringWithUTF8String:filepath]];

    Message msg;
    MessageQueueWait(msgQ, &msg);
    usleep(1000 * 10);
}

- (void)namidi:(NAMidi *)namidi onParseFinish:(Sequence *)sequence
{
    [self.namidi play];
}

- (void)namidi:(NAMidi *)namidi player:(Player *)player notifyEvent:(PlayerEvent)playerEvent
{
    if (PlayerEventReachEnd == playerEvent) {
        MessageQueuePost(msgQ, 0, NULL);
    }
}

@end
