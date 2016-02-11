//
//  ApplicationController.m
//  NAMIDI
//
//  Created by abechan on 2/11/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ApplicationController.h"
#import "AboutWindowController.h"
#import "PreferenceWindowController.h"
#import "WelcomeWindowController.h"
#import "GettingStartedWindowController.h"

@interface ApplicationController ()
@property (strong, nonatomic) WelcomeWindowController *welocomeWC;
@property (strong, nonatomic) GettingStartedWindowController *gettingStartedWC;
@property (strong, nonatomic) AboutWindowController *aboutWC;
@property (strong, nonatomic) PreferenceWindowController *preferenceWC;
@end


@implementation ApplicationController

static ApplicationController* _sharedInstance = nil;

+ (ApplicationController *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[ApplicationController alloc] init];
    });
    return _sharedInstance;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.aboutWC = [[AboutWindowController alloc] init];
        self.preferenceWC = [[PreferenceWindowController alloc] init];
        
    }
    return self;
}

- (void)showWelcomeWindow
{
    self.welocomeWC = [[WelcomeWindowController alloc] init];
    [_welocomeWC showWindow:self];
}

- (void)closeWelcomeWindow
{
    [_welocomeWC close];
}

- (void)showGettingStartedWindow
{
    self.gettingStartedWC = [[GettingStartedWindowController alloc] init];
    [_gettingStartedWC showWindow:self];
}

- (void)showAboutWindow
{
    [_aboutWC showWindow:self];
}

- (void)showPreferenceWindow
{
    [_preferenceWC showWindow:self];
}

@end
