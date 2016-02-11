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

@interface ApplicationController ()
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

- (void)showAboutWindow
{
    [_aboutWC showWindow:self];
}

- (void)showPreferenceWindow
{
    [_preferenceWC showWindow:self];
}

@end
