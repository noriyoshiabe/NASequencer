//
//  AppDelegate.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "AppDelegate.h"
#import "WelcomeWindowController.h"

@interface AppDelegate ()
@property (strong, nonatomic) WelcomeWindowController *welcomeWC;
@property (nonatomic) BOOL inLaunchOrReopenProcess;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    
    self.inLaunchOrReopenProcess = YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    // Insert code here to tear down your application
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    [self openWelcomeWindow];
    self.inLaunchOrReopenProcess = NO;
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    return NO;
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename
{
    [self closeWelcomeWindow];
    return NO;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
    self.inLaunchOrReopenProcess = YES;
    return YES;
}

- (void)openWelcomeWindow
{
    if (self.inLaunchOrReopenProcess && ![NSApplication sharedApplication].keyWindow) {
        self.welcomeWC = [[WelcomeWindowController alloc] init];
        [self.welcomeWC showWindow:self];
    }
}

- (void)closeWelcomeWindow
{
    if (self.welcomeWC) {
        [self.welcomeWC.window close];
        self.welcomeWC = nil;
    }
}

@end
