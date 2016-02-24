//
//  NAMidiRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NAMidiRepresentation.h"

@interface NAMidiRepresentation () {
    NAMidi *_namidi;
}

@end

@implementation NAMidiRepresentation

- (instancetype)initWithNAMidi:(NAMidi *)namidi
{
    self = [super init];
    if (self) {
        _namidi = namidi;
        _sequence = [[SequenceRepresentation alloc] initWithSequence:NAMidiGetSequence(_namidi)];
        _player = [[PlayerRepresentation alloc] initWithPlayer:NAMidiGetPlayer(_namidi)];
    }
    return self;
}

@end
