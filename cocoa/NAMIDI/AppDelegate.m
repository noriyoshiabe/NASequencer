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

@interface AppDelegate () {
    BOOL _inLaunchOrReopenProcess;
}
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{@"NSApplicationCrashOnExceptions":@YES}];
    
    [AppController initialize];
    
    _inLaunchOrReopenProcess = YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    [AppController finalize];
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    if (_inLaunchOrReopenProcess && AppController.needShowWelcome) {
        [AppController showWelcomeWindow];
    }
    _inLaunchOrReopenProcess = NO;
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
    _inLaunchOrReopenProcess = YES;
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

- (IBAction)officialSite:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://nasequencer.com"]];
}

- (IBAction)customerSupport:(id)sender
{
    NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
    [[NSHelpManager sharedHelpManager] openHelpAnchor:@"support" inBook:bookName];
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
