//
//  PlayerRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PlayerRepresentation.h"

@interface PlayerRepresentation () {
    Player *_player;
    NSHashTable *_observers;
}

- (void)onNotifyClock:(int)tick usec:(int64_t)usec location:(Location)location;
- (void)onNotifyEvent:(PlayerEvent)event;
- (void)onSendNoteOn:(NoteEvent *)event;
- (void)onSendNoteOff:(NoteEvent *)event;
@end

static void onNotifyClock(void *receiver, int tick, int64_t usec, Location location)
{
    PlayerRepresentation *player = (__bridge PlayerRepresentation *)receiver;
    [player onNotifyClock:tick usec:usec location:location];
}

static void onNotifyEvent(void *receiver, PlayerEvent event)
{
    PlayerRepresentation *player = (__bridge PlayerRepresentation *)receiver;
    [player onNotifyEvent:event];
}

static void onSendNoteOn(void *receiver, NoteEvent *event)
{
    PlayerRepresentation *player = (__bridge PlayerRepresentation *)receiver;
    [player onSendNoteOn:event];
}

static void onSendNoteOff(void *receiver, NoteEvent *event)
{
    PlayerRepresentation *player = (__bridge PlayerRepresentation *)receiver;
    [player onSendNoteOff:event];
}

static PlayerObserverCallbacks callbacks = {onNotifyClock, onNotifyEvent, onSendNoteOn, onSendNoteOff};

@implementation PlayerRepresentation

- (instancetype)initWithPlayer:(Player *)player
{
    self = [super init];
    if (self) {
        _observers = [NSHashTable weakObjectsHashTable];
        
        _player = player;
        PlayerAddObserver(_player, (__bridge void *)self, &callbacks);
    }
    return self;
}

- (void)dealloc
{
    if(_player) {
        PlayerRemoveObserver(_player, (__bridge void *)self);
        PlayerDestroy(_player);
    }
}

- (void)addObserver:(id<PlayerRepresentationObserver>)observer
{
    [_observers addObject:observer];
}

- (void)removeObserver:(id<PlayerRepresentationObserver>)observer
{
    [_observers removeObject:observer];
}

- (void)onNotifyClock:(int)tick usec:(int64_t)usec location:(Location)location
{
    [NSThread performBlockOnMainThread:^{
        for (id<PlayerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(player:onNotifyClock:usec:location:)]) {
                [observer player:self onNotifyClock:tick usec:usec location:location];
            }
        }
    }];
}

- (void)onNotifyEvent:(PlayerEvent)event
{
    [NSThread performBlockOnMainThread:^{
        for (id<PlayerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(player:onNotifyEvent:)]) {
                [observer player:self onNotifyEvent:event];
            }
        }
    }];
}

- (void)onSendNoteOn:(NoteEvent *)event
{
    [NSThread performBlockOnMainThread:^{
        for (id<PlayerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(player:onSendNoteOn:)]) {
                [observer player:self onSendNoteOn:event];
            }
        }
    }];
}

- (void)onSendNoteOff:(NoteEvent *)event
{
    [NSThread performBlockOnMainThread:^{
        for (id<PlayerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(player:onSendNoteOff:)]) {
                [observer player:self onSendNoteOff:event];
            }
        }
    }];
}

- (int64_t)usec
{
    return PlayerGetUsec(_player);
}

- (int)tick
{
    return PlayerGetTick(_player);
}

- (Location)location
{
    return PlayerGetLocation(_player);
}

- (float)tempo
{
    return PlayerGetTempo(_player);
}

- (TimeSign)timeSign
{
    return PlayerGetTimeSign(_player);
}

- (bool)isPlaying
{
    return PlayerIsPlaying(_player);
}

- (void)playPause
{
    PlayerPlayPause(_player);
}

- (void)rewind
{
    PlayerRewind(_player);
}

- (void)forward
{
    PlayerForward(_player);
}

- (void)backward
{
    PlayerBackWard(_player);
}

@end