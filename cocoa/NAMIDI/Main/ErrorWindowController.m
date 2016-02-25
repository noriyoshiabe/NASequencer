//
//  ErrorWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/20/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ErrorWindowController.h"
#import "Color.h"
#import "ParseErrorCode.h"

@interface ErrorWindowController () <NAMidiRepresentationObserver>
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
    
    [_namidi addObserver:self];
    [self update];
}

- (void)dealloc
{
    [_namidi removeObserver:self];
}

- (void)update
{
    NSMutableString *string = [NSMutableString string];
    for (ParseErrorRepresentation *error in _namidi.parseInfo.errors) {
        if (0 < string.length) {
            [string appendString:@"\n"];
        }
        [string appendString:[self formattedStringWithParseError:error]];
    }
    _errorTextField.stringValue = string;
}

- (NSString *)formattedStringWithParseError:(ParseErrorRepresentation *)error
{
    // TODO Actual formatting and localization
    
    return [NSString stringWithFormat:@"[ERROR:%d] %s %@ - %@:%d:%d",
            error.code, ParseErrorCode2String(error.code), [error.infos componentsJoinedByString:@", "],
            error.filepath.lastPathComponent, error.line, error.column];
}

- (IBAction)closeButtonPressed:(id)sender
{
    [self close];
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    [self update];
}

@end
