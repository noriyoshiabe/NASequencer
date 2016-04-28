//
//  AppDelegate.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "AppDelegate.h"
#import "ApplicationController.h"
#import "IAP.h"

@interface AppDelegate ()
@property (nonatomic) BOOL inLaunchOrReopenProcess;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [AppController initialize];
    [[IAP sharedInstance] initialize];
    self.inLaunchOrReopenProcess = YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    [[IAP sharedInstance] finalize];
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    if (self.inLaunchOrReopenProcess && AppController.needShowWelcome) {
        [AppController showWelcomeWindow];
    }
    self.inLaunchOrReopenProcess = NO;
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    return NO;
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename
{
    [AppController closeWelcomeWindow];
    return NO;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
    self.inLaunchOrReopenProcess = YES;
    return YES;
}

#pragma mark Menu Action

- (IBAction)preferenceButtonTapped:(id)sender
{
    [AppController showPreferenceWindow];
}

- (IBAction)newDocument:(id)sender
{
    [AppController createDocument];
}

- (IBAction)showWelcomeWindow:(id)sender
{
    [AppController showWelcomeWindow];
}

- (IBAction)showEventList:(id)sender
{
    // Suppress warning
}

- (IBAction)filterNoteEvent:(id)sender
{
    // Suppress warning
}

- (IBAction)filterControlEvent:(id)sender
{
    // Suppress warning
}

#pragma Default Menu Title

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if (@selector(showEventList:) == menuItem.action) {
        menuItem.title = NSLocalizedString(@"MainMenu_ShowEventList", @"Show Event List");
        return NO;
    }
    else if (@selector(filterNoteEvent:) == menuItem.action) {
        menuItem.title = NSLocalizedString(@"MainMenu_FilterNoteEvent", @"Filter Note Event");
        return NO;
    }
    else if (@selector(filterControlEvent:) == menuItem.action) {
        menuItem.title = NSLocalizedString(@"MainMenu_FilterControlEvent", @"Filter Control Event");
        return NO;
    }
    
    return YES;
}

@end
