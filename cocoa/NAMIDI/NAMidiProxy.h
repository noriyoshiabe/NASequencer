//
//  NAMidiProxy.h
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/26.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#ifndef NAMIDI_NAMidiProxy_h
#define NAMIDI_NAMidiProxy_h

#import <Foundation/Foundation.h>
#import "NAMidi.h"

@protocol NAMidiProxyDelegate;

@interface NAMidiProxy : NSObject
- (void)addDelegate:(id<NAMidiProxyDelegate>) delegate;
- (void)removeDelegate:(id<NAMidiProxyDelegate>) delegate;
- (void)setFile:(NSURL *)url;
- (void)parse;
- (void)play;
- (void)stop;
- (void)playPause;
- (void)rewind;
- (void)forward;
- (void)backward;
@end

@class ParseContextAdapter;
@class PlayerContextAdapter;

@protocol NAMidiProxyDelegate <NSObject>
@required
- (void)onParseFinished:(NAMidiProxy *)namidi context:(ParseContextAdapter *)context;
- (void)onPlayerContextChanged:(NAMidiProxy *)namidi context:(PlayerContextAdapter *)context;
@end

#endif
