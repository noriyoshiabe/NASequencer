#import "Controller.h"
#include "MessageQueue.h"

typedef enum {
    MessageKindParseFinish,
    MessageKindReachEnd,
} MessageKind;

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

- (void)run:(const char *)filepath repeat:(BOOL)repeat
{
    self.namidi.filepath = [NSString stringWithUTF8String:filepath];
    [self.namidi parse];

    bool exit = false;

    while (!exit) {
        Message msg;
        MessageQueueWait(msgQ, &msg);

        switch (msg.kind) {
        case MessageKindParseFinish:
            [self.namidi play];
            break;
        case MessageKindReachEnd:
            if (repeat) {
                [self.namidi rewind];
                [self.namidi play];
            }
            else {
                exit = true;
            }
            break;
        }
    }

    usleep(1000 * 10);
}

- (void)write:(const char *)filepath outfile:(const char *)outfile
{
    self.namidi.filepath = [NSString stringWithUTF8String:filepath];
    [self.namidi parse];

    Message msg;
    MessageQueueWait(msgQ, &msg);

    [self.namidi.sequence serialize:[NSString stringWithUTF8String:outfile]];
}

- (void)namidi:(NAMidi *)namidi onParseFinish:(Sequence *)sequence
{
    MessageQueuePost(msgQ, MessageKindParseFinish, NULL);
}

- (void)namidi:(NAMidi *)namidi player:(Player *)player notifyEvent:(PlayerEvent)playerEvent
{
    if (PlayerEventReachEnd == playerEvent) {
        MessageQueuePost(msgQ, MessageKindReachEnd, NULL);
    }
}

@end
