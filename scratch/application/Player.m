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

static void _PlayerClockSourceOnNotifyClock(void *receiver, int32_t tick, int32_t prevTick, int64_t usec, Location location)
{
    Player *self = (__bridge Player *)receiver;
    [self playerClockSource:self->clockSorce onNotifyClock:tick prevTick:prevTick usec:usec location:location];
}

static void _PlayerClockSourceOnNotifyEvent(void *receiver, PlayerClockSourceEvent event)
{
    Player *self = (__bridge Player *)receiver;
    [self playerClockSource:self->clockSorce onNotifyEvent:event];
}

static PlayerClockSourceCallbacks callbacks = {
    _PlayerClockSourceOnNotifyClock,
    _PlayerClockSourceOnNotifyEvent,
};

- (id)init
{
    if (self = [super init]) {
        clockSorce = PlayerClockSourceCreate(&callbacks, (__bridge void *)self);
    }
    return self;
}

- (void)dealloc
{
    PlayerClockSourceDestroy(clockSorce);
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

- (void)setSequence:(Sequence *)newSequence
{
    _sequence = newSequence;
    PlayerClockSourceSetTimeTable(clockSorce, _sequence.timeTable);
}

- (void)stop
{
    PlayerClockSourceStop(clockSorce);
}

- (void)play
{
    PlayerClockSourcePlay(clockSorce);
}

- (void)rewind
{
    PlayerClockSourceRewind(clockSorce);
}

- (void)forward
{
    PlayerClockSourceFoward(clockSorce);
}

- (void)backward
{
    PlayerClockSourceBackword(clockSorce);
}

- (void)playerClockSource:(PlayerClockSource *)clockSource onNotifyClock:(uint32_t)tick prevTick:(int32_t)prevTick usec:(int64_t)usec location:(Location)location
{
    printf("onNotifyClock() tick=%d prevTick=%d usec=%lld location=%d:%d:%d\n", tick, prevTick, usec, location.m, location.b, location.t);
    _tick = tick;
    _usec = usec;
    _location = location;

    if (_playing) {
        for (MidiEvent *event in [self.sequence eventsFrom:prevTick to:tick]) {
            // TODO
        }
    }
}

- (void)playerClockSource:(PlayerClockSource *)clockSource onNotifyEvent:(PlayerClockSourceEvent)event
{
    printf("onNotifyEvent() event=%s\n", PlayerClockSourceEvent2String(event));

    switch (event) {
    case PlayerClockSourceEventStop:
        _playing = NO;
        break;
    case PlayerClockSourceEventPlay:
        _playing = YES;
        break;
    case PlayerClockSourceEventRewind:
        break;
    case PlayerClockSourceEventForward:
        break;
    case PlayerClockSourceEventBackward:
        break;
    case PlayerClockSourceEventReachEnd:
        [self stop];
        [self rewind];
        break;
    }

    PlayerEvent playerEvent = [self playerEvent:event];
    if (PlayerEventUnknown != event) {
        [self.delegate player:self notifyEvent:playerEvent];
    }
}

- (PlayerEvent)playerEvent:(PlayerClockSourceEvent)event
{
    switch (event) {
    case PlayerClockSourceEventStop:
        return PlayerEventStop;
    case PlayerClockSourceEventPlay:
        return PlayerEventPlay;
    case PlayerClockSourceEventRewind:
        return PlayerEventRewind;
    case PlayerClockSourceEventForward:
        return PlayerEventForward;
    case PlayerClockSourceEventBackward:
        return PlayerEventBackward;

    case PlayerClockSourceEventReachEnd:
    default:
        return PlayerEventUnknown;
    }
}

@end
