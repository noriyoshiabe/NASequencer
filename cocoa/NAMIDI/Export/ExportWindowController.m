//
//  ExportWindowController.m
//  NAMIDI
//
//  Created by abechan on 3/1/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ExportWindowController.h"
#import "ExporterRepresentation.h"
#import "Color.h"

@interface ExportWindowController () <ExporterRepresentationDelegate> {
    ExporterRepresentation *_exporter;
}

@property (weak) IBOutlet NSView *backgroundView;
@property (weak) IBOutlet NSTextField *messageField;
@property (weak) IBOutlet NSProgressIndicator *progressIndicator;
@property (weak) IBOutlet NSTextField *completeLabel;
@end

@implementation ExportWindowController

- (NSString *)windowNibName
{
    return @"ExportWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    self.window.level = NSStatusWindowLevel;
    
    self.window.opaque = NO;
    self.window.backgroundColor = [NSColor clearColor];
    self.window.movableByWindowBackground = YES;
    
    _backgroundView.wantsLayer = YES;
    _backgroundView.layer.backgroundColor = [Color statusBackground].CGColor;
    _backgroundView.layer.cornerRadius = 10.0;
    _backgroundView.layer.masksToBounds = YES;
    
    _completeLabel.hidden = YES;
    
    NSString *messageFormat = NSLocalizedString(@"Export_Message", @"Exporting %@…");
    _messageField.stringValue = [NSString stringWithFormat:messageFormat, _file.filename];
    
    _exporter = [[ExporterRepresentation alloc] init];
    _exporter.delegate = self;
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        ExporterError error = [_exporter export:_file.url.path output:_outputUrl.path];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (ExporterErrorNoError != error) {
                [self close];
                
                NSString *informative = NSLocalizedString(@"Export_ExportErrorInformative", @"Could not export %@");
                
                NSAlert *alert = [[NSAlert alloc] init];
                alert.messageText = NSLocalizedString(@"Error", @"Error");
                alert.informativeText = [NSString stringWithFormat:informative, _file.filename];
                [alert addButtonWithTitle:NSLocalizedString(@"OK", @"OK")];;
                [alert runModal];
            }
            else {
                [self finish];
            }
        });
    });
}

- (void)exporter:(ExporterRepresentation *)exporter onProgress:(int)progress
{
    dispatch_async(dispatch_get_main_queue(), ^{
        _progressIndicator.doubleValue = progress;
    });
}

- (void)close
{
    [super close];
    [_delegate exportControllerDidClose:self];
}

- (void)finish
{
    _messageField.hidden = YES;
    _completeLabel.hidden = NO;
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        NSTimeInterval delay = [NSAnimationContext currentContext].duration + 0.1;
        [self performSelector:@selector(close) withObject:nil afterDelay:delay];
        self.window.animator.alphaValue = 0.0;
    });
}

@end
