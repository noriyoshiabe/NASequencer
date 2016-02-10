//
//  About.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "About.h"
#import "AboutWindowController.h"

@interface About () <AboutWindowControllerDelegate>
@property (strong, nonatomic) AboutWindowController *wc;
@end

@implementation About

static About* _sharedAbout = nil;

+ (About *)sharedAbout
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedAbout= [[About alloc] init];
    });
    return _sharedAbout;
}

- (void)showWindow
{
    if (!self.wc) {
        self.wc = [[AboutWindowController alloc] init];
        _wc.delegate = self;
        [_wc showWindow:self];
    }
}

#pragma mark AboutWindowControllerDelegate

- (void)aboutWindowControllerWillClose:(AboutWindowController *)controller
{
    self.wc = nil;
}

@end
