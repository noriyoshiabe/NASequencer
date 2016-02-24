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
}

@end

@implementation NAMidiRepresentation

- (instancetype)init
{
    self = [super init];
    if (self) {
        _namidi = NAMidiCreate();
    }
    return self;
}

- (void)addObserver:(id<NAMidiRepresentationObserver>)observer
{
    
}

- (void)removeObserver:(id<NAMidiRepresentationObserver>)observer
{
    
}

- (void)parse
{
    NAMidiSetWatchEnable(_namidi, true);
    NAMidiParse(_namidi, [_file.url.path UTF8String]);
    
    _sequence = [[SequenceRepresentation alloc] initWithSequence:NAMidiGetSequence(_namidi)];
    _player = [[PlayerRepresentation alloc] initWithPlayer:NAMidiGetPlayer(_namidi)];
}

- (void)dealloc
{
    NAMidiDestroy(_namidi);
}

@end
