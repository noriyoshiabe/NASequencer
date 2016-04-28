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

#import <Fabric/Fabric.h>
#import <Crashlytics/Crashlytics.h>

@interface AppDelegate () <IAPObserver>
@property (nonatomic) BOOL inLaunchOrReopenProcess;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{@"NSApplicationCrashOnExceptions":@YES}];
#if DEBUG
    [Crashlytics startWithAPIKey:@"36c32c7a4e94be63461f7a6015dc8b342058be4e"];
#else
    [Crashlytics startWithAPIKey:@"5ca7632975921be3a41bdf2b4544373f5f888ac4"];
#endif
    
    [AppController initialize];
    
    [[IAP sharedInstance] addObserver:self];
    [[IAP sharedInstance] initialize];
    
    self.inLaunchOrReopenProcess = YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    [[IAP sharedInstance] removeObserver:self];
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

#pragma mark IAPObserver

- (void)iap:(IAP *)iap didUpdateTransaction:(SKPaymentTransaction *)transaction
{
    switch (transaction.transactionState) {
        case SKPaymentTransactionStatePurchasing:
        case SKPaymentTransactionStateDeferred:
            break;
        case SKPaymentTransactionStateFailed:
        {
            NSString *informative = NSLocalizedString(@"Purchase_PurchaseFaildInformative", @"An error has occurred.\n%@ - %d");
            NSAlert *alert = [[NSAlert alloc] init];
            alert.messageText = NSLocalizedString(@"Purchase_Failed", @"Purchase Faild");
            alert.informativeText = [NSString stringWithFormat:informative, transaction.error.domain, transaction.error.code];
            [alert addButtonWithTitle:NSLocalizedString(@"Close", @"Close")];;
            [alert runModal];
        }
            break;
        case SKPaymentTransactionStatePurchased:
        case SKPaymentTransactionStateRestored:
            break;
    }
}

@end
