#pragma once

#import <Foundation/Foundation.h>

#import "Sequence.h"
#import "ParseError.h"
#import "Player.h"

@protocol NAMidiObserver;

@interface NAMidi : NSObject<PlayerDelegate>

@property (nonatomic, readonly) Sequence *sequence;
@property (nonatomic, readonly) Player *player;

- (void)addObserver:(id<NAMidiObserver>)observer;
- (void)removeObserver:(id<NAMidiObserver>)observer;
- (void)parse:(NSString *)filepath;

@end

@protocol NAMidiObserver <NSObject>
@optional
- (void)namidi:(NAMidi *)namidi onParseFinish:(Sequence *)sequence;
- (void)namidi:(NAMidi *)namidi onParseError:(NSString *)filepath line:(int)line column:(int)column error:(ParseError)error info:(const void *)info;
- (void)namidi:(NAMidi *)namidi player:(Player *)player notifyEvent:(PlayerEvent)playerEvent;
- (void)namidi:(NAMidi *)namidi player:(Player *)player didSendNoteOn:(NoteEvent *)noteEvent;
- (void)namidi:(NAMidi *)namidi player:(Player *)player didSendNoteOff:(NoteEvent *)noteEvent;
@end

