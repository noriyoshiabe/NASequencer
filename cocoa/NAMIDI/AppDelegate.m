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
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    [self openWelcomeWindow];
    return NO;
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename
{
    if (self.welcomeWC) {
        [self.welcomeWC.window close];
        self.welcomeWC = nil;
    }
    
    return NO;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
    if (flag) {
        return NO;
    }
    else {
        [self openWelcomeWindow];
        return YES;
    }
}

- (void)openWelcomeWindow
{
    self.welcomeWC = [[WelcomeWindowController alloc] init];
    [self.welcomeWC showWindow:self];
}

@end
