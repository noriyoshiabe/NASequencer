#import "Player.h"
#import "PlayerClockSource.h"
#import "Mixer.h"

@interface Player() {
    PlayerClockSource *clockSorce;
    bool _playing;
    int64_t _usec;
    int32_t _tick;
    Location _location;
    NSMutableSet *_playingNoteEvents;

    int _index;
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
        _playingNoteEvents = [NSMutableSet set];
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

- (void)sendNoteOn:(NoteEvent *)event
{
    [[Mixer sharedInstance] sendNoteOn:event];

    [_playingNoteEvents addObject:event];

    if ([self.delegate respondsToSelector:@selector(player:didSendNoteOn:)]) {
        [self.delegate player:self didSendNoteOn:event];
    }
}

- (void)sendNoteOff:(NoteEvent *)event
{
    [[Mixer sharedInstance] sendNoteOff:event];

    [_playingNoteEvents removeObject:event];

    if ([self.delegate respondsToSelector:@selector(player:didSendNoteOff:)]) {
        [self.delegate player:self didSendNoteOff:event];
    }
}

- (void)sendAllNoteOff
{
    for (NoteEvent *event in [self.playingNoteEvents allObjects]) {
        [self sendNoteOff:event];
    }

    [[Mixer sharedInstance] sendAllNoteOff];
}

- (void)scanNoteOffFrom:(int32_t)from to:(int32_t)to
{
    for (NoteEvent *event in [self.playingNoteEvents allObjects]) {
        if (from <= event.offTick && event.offTick < to) {
            [self sendNoteOff:event];
        }
    }
}

- (void)sendSound:(SoundEvent *)event
{
    [[Mixer sharedInstance] sendSound:event];

    if ([self.delegate respondsToSelector:@selector(player:didSendSound:)]) {
        [self.delegate player:self didSendSound:event];
    }
}

- (void)playerClockSource:(PlayerClockSource *)clockSource onNotifyClock:(uint32_t)tick prevTick:(int32_t)prevTick usec:(int64_t)usec location:(Location)location
{
    _tick = tick;
    _usec = usec;
    _location = location;

    if (_playing && prevTick < tick) {
        [self scanNoteOffFrom:prevTick to:tick];

        NSArray *events = self.sequence.events;
        int count = [events count];
        for (; _index < count; ++_index) {
            MidiEvent *event = [events objectAtIndex:_index];
            if (prevTick <= event.tick && event.tick < tick) {
                switch (event.type) {
                case MidiEventTypeNote:
                    [self sendNoteOn:(NoteEvent *)event];
                    break;
                case MidiEventTypeSound:
                    [self sendSound:(SoundEvent *)event];
                    break;
                default:
                    break;
                }
            }
            else if (tick <= event.tick) {
                break;
            }
        }
    }
}

- (void)seekIndexForward
{
    NSArray *events = self.sequence.events;
    int count = [events count];

    for (; _index < count; ++_index) {
        MidiEvent *event = [events objectAtIndex:_index];
        if (_tick <= event.tick) {
            break;
        }
    }
}

- (void)seekIndexBackward
{
    NSArray *events = self.sequence.events;
    int count = [events count];

    for (_index = MIN(MAX(0, _index - 1), count - 1); 0 < _index; --_index) {
        MidiEvent *event = [events objectAtIndex:_index];
        if (_tick >= event.tick) {
            break;
        }
    }
}

- (void)playerClockSource:(PlayerClockSource *)clockSource onNotifyEvent:(PlayerClockSourceEvent)event
{
    switch (event) {
    case PlayerClockSourceEventStop:
        [self sendAllNoteOff];
        _playing = NO;
        break;
    case PlayerClockSourceEventPlay:
        _playing = YES;
        break;
    case PlayerClockSourceEventRewind:
        [self sendAllNoteOff];
        _index = 0;
        break;
    case PlayerClockSourceEventForward:
        [self sendAllNoteOff];
        [self seekIndexForward];
        break;
    case PlayerClockSourceEventBackward:
        [self sendAllNoteOff];
        [self seekIndexBackward];
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
