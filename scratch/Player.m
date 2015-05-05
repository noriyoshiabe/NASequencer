#import "Player.h"
#import "PlayerClockSource.h"

@interface Player() {
    PlayerClockSource *clockSorce;
    bool _playing;
    int64_t _usec;
    int32_t _tick;
    Location _location;
}

@end

@implementation Player

static void _PlayerClockSourceOnSupplyClock(void *receiver, int64_t usec, uint32_t tick, Location location)
{
    Player *self = (__bridge Player *)receiver;
    [self playerClockSource:self->clockSorce onSupplyClock:usec tick:tick location:location];
}

static void _PlayerClockSourceOnNotifyEvent(void *receiver, PlayerClockSourceEvent event)
{
    Player *self = (__bridge Player *)receiver;
    [self playerClockSource:self->clockSorce onNotifyEvent:event];
}

static PlayerClockSourceCallbacks callbacks {
    _PlayerClockSourceOnSupplyClock,
    _PlayerClockSourceOnNotifyEvent,
};

- (id)init
{
    if (self = [super init]) {
        clockSorce = PlayerClockSourceCreate(&callbacks);
    }
    return self;
}

- (bool)playing
{
    return _playing;
}

- (int64_t)usec
{
    return _usec;
}

- (int32_t)tick
{
    return _tick;
}

- (Location)location
{
    return _location;
}

- (void)playerClockSource:(PlayerClockSourceCreate *)clockSource onSupplyClock:(int64_t)usec tick:(uint32_t)tick location:(Location)location
{
    printf("onSupplyClock() usec=%d tick=%d location=%d:%d:%d\n", usec, tick, location.m, location.b, location.t);
}

- (void)playerClockSource:(PlayerClockSourceCreate *)clockSource onNotifyEvent:(PlayerClockSourceEvent)event
{
    printf("onNotifyEvent() event=%s\n", PlayerClockSourceEvent2String(event));
}

@end
