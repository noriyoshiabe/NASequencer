//
//  MainWindowContext.m
//  NAMIDI
//
//  Created by abechan on 4/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MainWindowContext.h"

@implementation MainWindowContext

- (instancetype)init
{
    self = [super init];
    if (self) {
        _autoScrollEnabled = YES;
    }
    return self;
}

@end
