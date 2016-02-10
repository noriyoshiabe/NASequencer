//
//  Preference.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Preference.h"
#import "PreferenceWindowController.h"
#import "GeneralViewController.h"
#import "SynthesizerViewController.h"
#import "PurchaseViewController.h"

@interface Preference () <PreferenceWindowControllerDelegate>
@property (strong, nonatomic) PreferenceWindowController *wc;
@end

@implementation Preference

static Preference* _sharedInstance = nil;

+ (Preference *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[Preference alloc] init];
    });
    return _sharedInstance;
}

- (void)showWindow
{
    if (!self.wc) {
        self.wc = [[PreferenceWindowController alloc] init];
        _wc.delegate = self;
        
        [_wc addViewController:[[GeneralViewController alloc] init]];
        [_wc addViewController:[[SynthesizerViewController alloc] init]];
        [_wc addViewController:[[PurchaseViewController alloc] init]];
        [_wc showWindow:self];
    }
}

#pragma mark PreferenceWindowControllerDelegate

- (void)preferenceWindowControllerWillClose:(PreferenceWindowController *)controller
{
    self.wc = nil;
}

@end
