//
//  NAMidiRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "SequenceRepresentation.h"
#import "PlayerRepresentation.h"
#import "NAMidi.h"

@interface NAMidiRepresentation : NSObject
@property (readonly, nonatomic) SequenceRepresentation *sequence;
@property (readonly, nonatomic) PlayerRepresentation *player;
- (instancetype)initWithNAMidi:(NAMidi *)namidi;
@end
