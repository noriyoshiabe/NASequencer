//
//  ErrorWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/20/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ErrorWindowController.h"
#import "Color.h"

@interface ErrorWindowController ()
@property (weak) IBOutlet NSTextField *errorTextField;
@end

@implementation ErrorWindowController

- (NSString *)windowNibName
{
    return @"ErrorWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    self.window.opaque = NO;
    self.window.backgroundColor = [NSColor clearColor];
    self.window.movableByWindowBackground = YES;
    self.window.contentView.wantsLayer = YES;
    self.window.contentView.layer.backgroundColor = [Color statusBackground].CGColor;
    self.window.contentView.layer.cornerRadius = 10.0;
    self.window.contentView.layer.masksToBounds = YES;
}

- (IBAction)closeButtonPressed:(id)sender
{
    [self close];
}

@end
