//
//  PresetSelectionWindowController.m
//  NAMIDI
//
//  Created by abechan on 4/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PresetSelectionWindowController.h"

@interface PresetSelectionWindowController () <NSWindowDelegate>

@end

@implementation PresetSelectionWindowController

- (NSString *)windowNibName
{
    return @"PresetSelectionWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
}

- (IBAction)okButtonPressed:(id)sender
{
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseCancel];
}

#pragma NSWindowDelegate

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    
}

@end
