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

#define HEIGHT_WITHOUT_TEXT 54

@interface ErrorWindowController () <NAMidiRepresentationObserver, NSWindowDelegate>
@property (weak) IBOutlet NSView *backGroundView;
@property (unsafe_unretained) IBOutlet NSTextView *textView;
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
    
    _backGroundView.wantsLayer = YES;
    _backGroundView.layer.backgroundColor = [Color statusBackground].CGColor;
    _backGroundView.layer.cornerRadius = 10.0;
    _backGroundView.layer.masksToBounds = YES;
    
    _textView.font = [NSFont systemFontOfSize:12.0];
    _textView.textColor = [NSColor whiteColor];
    
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
    _textView.string = string;
    
    [_textView.textContainer setLineFragmentPadding:0.0];
    [_textView.layoutManager glyphRangeForTextContainer:_textView.textContainer];
    CGRect contentRect = [_textView.layoutManager usedRectForTextContainer:_textView.textContainer];
    
    CGFloat height = MIN(500.0, contentRect.size.height + HEIGHT_WITHOUT_TEXT);
    CGFloat offsetY = self.window.frame.size.height - height;
    [self.window setFrame:CGRectMake(self.window.frame.origin.x, self.window.frame.origin.y + offsetY,
                                     self.window.frame.size.width, height) display:YES];
}

- (NSString *)formattedStringWithParseError:(ParseErrorRepresentation *)error
{
    // TODO Actual formatting and localization
    
    NSString *message = [error.formattedString stringByReplacingOccurrencesOfString:NSUserHomeDirectory() withString:@"~"];
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"^\\[ERROR:[0-9]*\\] " options:0 error:nil];
    return [regex stringByReplacingMatchesInString:message options:0 range:NSMakeRange(0, message.length) withTemplate:@""];
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
