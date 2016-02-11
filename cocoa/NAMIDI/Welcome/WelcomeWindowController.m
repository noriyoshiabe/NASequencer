//
//  WelcomeWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "WelcomeWindowController.h"
#import "WelcomeView.h"
#import "Application.h"
#import "ApplicationController.h"

@interface WelcomeWindowController () <WelcomeViewDelegate>
@property (weak) IBOutlet WelcomeView *welcomeView;
@end

@implementation WelcomeWindowController

- (instancetype)init
{
    return [self initWithWindowNibName:@"WelcomeWindowController"];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    self.windowFrameAutosaveName = @"WelcomeWindowFrame";
    
    _welcomeView.delegate = self;
    _welcomeView.recentFiles = [NSDocumentController sharedDocumentController].recentDocumentURLs;
}

#pragma mark WelcomeViewDelegate

- (void)welcomeView:(WelcomeView *)view closeButtonTapped:(id)sender
{
    [self close];
}

- (void)welcomeView:(WelcomeView *)view gettingStartedButtonTapped:(id)sender
{
    [[ApplicationController sharedInstance] showGettingStartedWindow];
}

- (void)welcomeView:(WelcomeView *)view listenToExampleButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
}

- (void)welcomeView:(WelcomeView *)view createNewDocumentButtonTapped:(id)sender
{
    [[Application sharedApplication] createDocument];
}

- (void)welcomeView:(WelcomeView *)view helpButtonTapped:(id)sender
{
    NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
    [[NSHelpManager sharedHelpManager] openHelpAnchor:@"top" inBook:bookName];
}

- (void)welcomeView:(WelcomeView *)view preferenceButtonTapped:(id)sender
{
    [[ApplicationController sharedInstance] showPreferenceWindow];
}

- (void)welcomeView:(WelcomeView *)view openOtherDocumentButtonTapped:(id)sender
{
    [[Application sharedApplication] openDocument];
}

- (void)welcomeView:(WelcomeView *)view showWelcomeWhenStartsToggled:(NSButton *)sender
{
    NSLog(@"%s %s", NSOnState == sender.state ? "ON" : "OFF", __func__);
}

- (void)welcomeView:(WelcomeView *)view recentTableViewSelectionChanged:(id)sender selectedRow:(NSInteger)row
{
    [[Application sharedApplication] openDocumentWithContentsOfURL:_welcomeView.recentFiles[row]];
}

@end
