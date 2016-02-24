//
//  PlayerRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Player.h"

@interface PlayerRepresentation : NSObject
@property (readonly, nonatomic) int64_t usec;
@property (readonly, nonatomic) int tick;
@property (readonly, nonatomic) Location location;
@property (readonly, nonatomic) float tempo;
@property (readonly, nonatomic) TimeSign timeSign;
- (instancetype)initWithPlayer:(Player *)player;
@end
