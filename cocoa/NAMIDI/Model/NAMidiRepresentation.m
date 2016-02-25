//
//  NAMidiRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NAMidiRepresentation.h"
#import "NAMidi.h"

@interface NAMidiRepresentation () {
    NAMidi *_namidi;
    NSHashTable *_observers;
}

- (void)onBeforeParse:(BOOL)fileChanged;
- (void)onParseFinish:(Sequence *)sequence info:(ParseInfo *)info;
@end

static void onBeforeParse(void *receiver, bool fileChanged)
{
    NAMidiRepresentation *namidi = (__bridge NAMidiRepresentation *)receiver;
    [namidi onBeforeParse:fileChanged];
}

static void onParseFinish(void *receiver, Sequence *sequence, ParseInfo *info)
{
    NAMidiRepresentation *namidi = (__bridge NAMidiRepresentation *)receiver;
    [namidi onParseFinish:sequence info:info];
}

static NAMidiObserverCallbacks callbacks = {onBeforeParse, onParseFinish};

@implementation NAMidiRepresentation

- (instancetype)init
{
    self = [super init];
    if (self) {
        _observers = [NSHashTable weakObjectsHashTable];
        
        _namidi = NAMidiCreate();
        NAMidiAddObserver(_namidi, (__bridge void *)self, &callbacks);
        
        _player = [[PlayerRepresentation alloc] initWithPlayer:NAMidiGetPlayer(_namidi)];
    }
    return self;
}

- (void)addObserver:(id<NAMidiRepresentationObserver>)observer
{
    [_observers addObject:observer];
}

- (void)removeObserver:(id<NAMidiRepresentationObserver>)observer
{
    [_observers removeObject:observer];
}

- (void)onBeforeParse:(BOOL)fileChanged
{
    for (id<NAMidiRepresentationObserver> observer in _observers) {
        [observer namidiWillParse:self fileChanged:fileChanged];
    }
}

- (void)onParseFinish:(Sequence *)sequence info:(ParseInfo *)info
{
    _sequence = [[SequenceRepresentation alloc] initWithSequence:NAMidiGetSequence(_namidi)];
    _parseInfo = [[ParseInfoRepresentation alloc] initWithParseInfo:NAMidiGetParseInfo(_namidi)];
    
    for (id<NAMidiRepresentationObserver> observer in _observers) {
        [observer namidiDidParse:self sequence:_sequence parseInfo:_parseInfo];
    }
}

- (void)parse
{
    NAMidiSetWatchEnable(_namidi, true);
    NAMidiParse(_namidi, [_file.url.path UTF8String]);
}

- (void)dealloc
{
    if(_namidi) {
        NAMidiRemoveObserver(_namidi, (__bridge void *)self);
        NAMidiDestroy(_namidi);
    }
}

@end
