//
//  PlayerRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Player.h"

@class PlayerRepresentation;
@protocol PlayerRepresentationObserver <NSObject>
@optional
- (void)player:(PlayerRepresentation *)player onNotifyClock:(int)tick usec:(int64_t)usec location:(Location)location;
- (void)player:(PlayerRepresentation *)player onNotifyEvent:(PlayerEvent)event;
- (void)player:(PlayerRepresentation *)player onSendNoteOn:(NoteEvent *)event;
- (void)player:(PlayerRepresentation *)player onSendNoteOff:(NoteEvent *)event;
@end

@interface PlayerRepresentation : NSObject
@property (readonly, nonatomic) int64_t usec;
@property (readonly, nonatomic) int tick;
@property (readonly, nonatomic) Location location;
@property (readonly, nonatomic) float tempo;
@property (readonly, nonatomic) TimeSign timeSign;
@property (readonly, nonatomic) bool isPlaying;
- (instancetype)initWithPlayer:(Player *)player;
- (void)addObserver:(id<PlayerRepresentationObserver>)observer;
- (void)removeObserver:(id<PlayerRepresentationObserver>)observer;
- (void)playPause;
- (void)stop;
- (void)rewind;
- (void)forward;
- (void)backward;
- (void)seek:(Location)location;
@end
