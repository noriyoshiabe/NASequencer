//
//  AppDelegate.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "AppDelegate.h"
#import "Welcome.h"

@interface AppDelegate ()
@property (nonatomic) BOOL inLaunchOrReopenProcess;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    self.inLaunchOrReopenProcess = YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    if (self.inLaunchOrReopenProcess && ![NSApplication sharedApplication].keyWindow) {
        [[Welcome sharedInstance] showWindow];
    }
    self.inLaunchOrReopenProcess = NO;
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    return NO;
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename
{
    [[Welcome sharedInstance] closeWindow];
    return NO;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
    self.inLaunchOrReopenProcess = YES;
    return YES;
}

@end
