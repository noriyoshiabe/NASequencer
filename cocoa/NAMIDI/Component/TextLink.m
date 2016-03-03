//
//  TextLink.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "TextLink.h"

@implementation TextLink

- (void)mouseUp:(NSEvent *)theEvent
{
    [self sendAction:[self action] to:[self target]];
}

@end
