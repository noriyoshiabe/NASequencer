//
//  NAMidiRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NAMidiRepresentation.h"
#import "NAMidi.h"
#import "ObserverList.h"

@interface PlayerRepresentation (Finalize)
- (void)finalize;
@end

@interface MixerRepresentation (Finalize)
- (void)finalize;
@end

@interface NAMidiRepresentation () {
    NAMidi *_namidi;
    ObserverList *_observers;
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
        _observers = [[ObserverList alloc] init];
        
        _namidi = NAMidiCreate();
        NAMidiAddObserver(_namidi, (__bridge void *)self, &callbacks);
        
        _player = [[PlayerRepresentation alloc] initWithPlayer:NAMidiGetPlayer(_namidi)];
        _mixer = [[MixerRepresentation alloc] initWithMixer:NAMidiGetMixer(_namidi)];
    }
    return self;
}

- (void)dealloc
{
    if(_namidi) {
        [_player finalize];
        [_mixer finalize];
        
        NAMidiRemoveObserver(_namidi, (__bridge void *)self);
        NAMidiDestroy(_namidi);
    }
}

- (void)addObserver:(id<NAMidiRepresentationObserver>)observer
{
    [_observers addObserver:observer];
}

- (void)removeObserver:(id<NAMidiRepresentationObserver>)observer
{
    [_observers removeObserver:observer];
}

- (void)onBeforeParse:(BOOL)fileChanged
{
    [NSThread performBlockOnMainThread:^{
        for (id<NAMidiRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(namidiWillParse:fileChanged:)]) {
                [observer namidiWillParse:self fileChanged:fileChanged];
            }
        }
    }];
}

- (void)onParseFinish:(Sequence *)sequence info:(ParseInfo *)info
{
    SequenceRepresentation *__sequence = [[SequenceRepresentation alloc] initWithSequence:NAMidiGetSequence(_namidi)];
    ParseInfoRepresentation *__parseInfo = [[ParseInfoRepresentation alloc] initWithParseInfo:NAMidiGetParseInfo(_namidi)];
    
    [NSThread performBlockOnMainThread:^{
        _sequence = __sequence;
        _parseInfo = __parseInfo;
        
        for (id<NAMidiRepresentationObserver> observer in _observers) {
            [observer namidiDidParse:self sequence:_sequence parseInfo:_parseInfo];
        }
    }];
}

- (void)parse
{
    NAMidiSetWatchEnable(_namidi, true);
    NAMidiSetIncludePath(_namidi, _includePath ? _includePath.UTF8String : NULL);
    NAMidiParse(_namidi, _file.url.path.UTF8String);
}

- (BOOL)hasError
{
    return 0 < _parseInfo.errors.count;
}

@end
