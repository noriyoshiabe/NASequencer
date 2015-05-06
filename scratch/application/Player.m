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
    if (self.sequence) {
        PlayerClockSourceStop(clockSorce);
    }
}

- (void)play
{
    if (self.sequence) {
        PlayerClockSourcePlay(clockSorce);
    }
}

- (void)playPause
{
    if (self.sequence) {
        if (self.playing) {
            [self stop];
        }
        else {
            [self play];
        }
    }
}

- (void)rewind
{
    if (self.sequence) {
        PlayerClockSourceRewind(clockSorce);
    }
}

- (void)forward
{
    if (self.sequence) {
        PlayerClockSourceFoward(clockSorce);
    }
}

- (void)backward
{
    if (self.sequence) {
        PlayerClockSourceBackword(clockSorce);
    }
}

- (void)playerClockSource:(PlayerClockSource *)clockSource onNotifyClock:(uint32_t)tick prevTick:(int32_t)prevTick usec:(int64_t)usec location:(Location)location
{
#if 0
    printf("onNotifyClock() tick=%d prevTick=%d usec=%lld location=%d:%d:%d\n", tick, prevTick, usec, location.m, location.b, location.t);
#endif

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
    if ([self.delegate respondsToSelector:@selector(player:notifyEvent:)]) {
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
        return PlayerEventReachEnd;
    }
}

+ (NSString *)playerEvent2String:(PlayerEvent)playerEvent
{
#define CASE(event) case event: return @#event
    switch (playerEvent) {
    CASE(PlayerEventStop);
    CASE(PlayerEventPlay);
    CASE(PlayerEventRewind);
    CASE(PlayerEventForward);
    CASE(PlayerEventBackward);
    CASE(PlayerEventReachEnd);
    }
    return @"Unknown player event";
#undef CASE
}

@end
