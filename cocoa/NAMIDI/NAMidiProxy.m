//
//  NAMidiProxy.m
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/26.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#import "NAMidiProxy.h"

typedef struct _ObserverBridge {
    NAType _;
    void *proxy;
} ObserverBridge;

@interface NAMidiProxy() {
    NAMidi *namidi;
    ObserverBridge *bridge;
}

@property (nonatomic, strong) NSHashTable *delegates;
@end

static void *__ObserverBridgeInit(void *_self, ...)
{
    ObserverBridge *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->proxy = va_arg(ap, void *);
    va_end(ap);
    return self;
}

static void __ObserverBridgeOnParseFinished(void *_self, NAMidi *sender, ParseContext *context)
{
    ObserverBridge *self = _self;
    NAMidiProxy *proxy = (__bridge NAMidiProxy *)self->proxy;
    for (id<NAMidiProxyDelegate> delegate in proxy.delegates) {
        [delegate onParseFinished:proxy context:context];
    }
}

static void __ObserverBridgeOnPlayerContextChanged(void *_self, NAMidi *sender, PlayerContext *context)
{
    ObserverBridge *self = _self;
    NAMidiProxy *proxy = (__bridge NAMidiProxy *)self->proxy;
    for (id<NAMidiProxyDelegate> delegate in proxy.delegates) {
        [delegate onPlayerContextChanged:proxy context:context];
    }
}

NADeclareVtbl(ObserverBridge, NAType, __ObserverBridgeInit, NULL, NULL, NULL, NULL, NULL, NULL);
NADeclareVtbl(ObserverBridge, NAMidiObserver, __ObserverBridgeOnParseFinished, __ObserverBridgeOnPlayerContextChanged);
NADeclareClass(ObserverBridge, NAType, NAMidiObserver);

@implementation NAMidiProxy

- (id)init
{
    if (self = [super init]) {
        self.delegates = [NSHashTable weakObjectsHashTable];
        namidi = NATypeNew(NAMidi);
        bridge = NATypeNew(ObserverBridge, self);
        NAMidiAddObserver(namidi, bridge);
    }
    return self;
}

- (void)dealloc
{
    NARelease(namidi);
    NARelease(bridge);
    self.delegates = nil;
}

- (void)addDelegate:(id<NAMidiProxyDelegate>)delegate
{
    [_delegates addObject:delegate];
}

- (void)removeDelegate:(id<NAMidiProxyDelegate>)delegate
{
    [_delegates removeObject:delegate];
}

- (void)setFile:(NSURL *)url
{
    NAMidiSetFile(namidi, (__bridge CFStringRef)url.path);
}

- (void)parse
{
    NAMidiParse(namidi);
}

- (void)play
{
    NAMidiPlay(namidi);
}

- (void)stop
{
    NAMidiStop(namidi);
}

- (void)playPause
{
    NAMidiPlayPause(namidi);
}

- (void)rewind
{
    NAMidiRewind(namidi);
}

- (void)forward
{
    NAMidiForward(namidi);
}

- (void)backward
{
    NAMidiBackward(namidi);
}

@end