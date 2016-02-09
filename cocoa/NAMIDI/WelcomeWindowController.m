//
//  WelcomeWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "WelcomeWindowController.h"

@interface WelcomeWindowController ()

@end

@implementation WelcomeWindowController

- (instancetype)init
{
    return [self initWithWindowNibName:@"WelcomeWindowController"];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (IBAction)closeButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
    [self close];
}

- (IBAction)gettingStartedButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)listenToExampleButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)createNewDocumentButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)helpButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)preferenceButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)openOtherDocumentButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)showWhelcomeWhenStartsToggled:(NSButton *)sender
{
    NSLog(@"%s %s", NSOnState == sender.state ? "ON" : "OFF", __func__);
}

@end
