//
//  NAMidiObserver.m
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/19.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#import "NAMidiObserver.h"

typedef struct _ObserverBridge {
    NAType _;
    void *observer;
} ObserverBridge;

static void *__ObserverBridgeInit(void *_self, ...)
{
    ObserverBridge *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->observer = va_arg(ap, void *);
    va_end(ap);
    return self;
}

static void __ObserverBridgeOnParseFinished(void *_self, NAMidi *sender, ParseContext *context)
{
    ObserverBridge *self = _self;
    NAMidiObserver *observer = (__bridge NAMidiObserver *)self->observer;
    [observer.delegate onParseFinished:sender context:context];
}

static void __ObserverBridgeOnPlayerContextChanged(void *_self, NAMidi *sender, PlayerContext *context)
{
    ObserverBridge *self = _self;
    NAMidiObserver *observer = (__bridge NAMidiObserver *)self->observer;
    [observer.delegate onPlayerContextChanged:sender context:context];
}

NADeclareVtbl(ObserverBridge, NAType, __ObserverBridgeInit, NULL, NULL, NULL, NULL, NULL, NULL);
NADeclareVtbl(ObserverBridge, NAMidiObserver, __ObserverBridgeOnParseFinished, __ObserverBridgeOnPlayerContextChanged);
NADeclareClass(ObserverBridge, NAType, NAMidiObserver);

@implementation NAMidiObserver

- (id)initWithDelegate:(id<NAMidiObserverDelegate>)delegate
{
    if (self = [super init]) {
        self.delegate = delegate;
        self.observerBridge = NATypeNew(ObserverBridge, self);
    }
    return self;
}

- (void)dealloc
{
    self.delegate = nil;
    NARelease(self.observerBridge);
}

@end