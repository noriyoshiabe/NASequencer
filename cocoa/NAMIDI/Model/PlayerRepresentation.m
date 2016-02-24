//
//  PlayerRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PlayerRepresentation.h"

@interface PlayerRepresentation () {
    Player *_player;
}

@end

@implementation PlayerRepresentation

- (instancetype)initWithPlayer:(Player *)player
{
    self = [super init];
    if (self) {
        _player = player;
    }
    return self;
}

- (int64_t)usec
{
    return PlayerGetUsec(_player);
}

- (int)tick
{
    return PlayerGetTick(_player);
}

- (Location)location
{
    return PlayerGetLocation(_player);
}

- (float)tempo
{
    return PlayerGetTempo(_player);
}

- (TimeSign)timeSign
{
    return PlayerGetTimeSign(_player);
}

@end
