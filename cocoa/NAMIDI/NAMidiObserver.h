//
//  NAMidiObserver.h
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/19.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#ifndef NAMIDI_NAMidiObserver_h
#define NAMIDI_NAMidiObserver_h

#import "NAMidi.h"

@protocol NAMidiObserverDelegate <NSObject>
@required
- (void)onParseFinished:(NAMidi *)namidi parseContext:(ParseContext *)parseContext;
- (void)onPlayingStateChanged:(NAMidi *)namidi playingState:(void *)playingState;
@end

@interface NAMidiObserver : NSObject
@property (nonatomic, weak) id<NAMidiObserverDelegate> delegate;
@property (nonatomic) void *observerBridge;
- (id)initWithDelegate:(id<NAMidiObserverDelegate>)delegate;
@end

#endif
