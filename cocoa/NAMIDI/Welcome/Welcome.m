//
//  Welcome.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Welcome.h"
#import "WelcomeWindowController.h"

@interface Welcome () <WelcomeWindowControllerDelegate>
@property (strong, nonatomic) WelcomeWindowController *wc;
@end


@implementation Welcome

static Welcome* _sharedInstance = nil;

+ (Welcome *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[Welcome alloc] init];
    });
    return _sharedInstance;
}

- (void)showWindow
{
    if (!self.wc) {
        self.wc = [[WelcomeWindowController alloc] init];
        _wc.delegate = self;
        [_wc showWindow:self];
    }
}

- (void)closeWindow
{
    if (self.wc) {
        [self.wc.window close];
        self.wc = nil;
    }
}

#pragma mark WelcomeWindowControllerDelegate

- (void)welcomeWindowControllerWillClose:(WelcomeWindowController *)controller
{
    self.wc = nil;
}

@end
