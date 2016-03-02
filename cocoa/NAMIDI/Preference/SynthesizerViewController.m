//
//  SynthesizerViewController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SynthesizerViewController.h"
#import "SynthesizerRowView.h"
#import "MidiSourceManagerRepresentation.h"

@import QuartzCore.CAMediaTimingFunction;

@interface SynthesizerViewController () <NSTableViewDataSource, NSTableViewDelegate, MidiSourceManagerRepresentationObserver, SynthesizerRowViewDelegate> {
    MidiSourceManagerRepresentation *_manager;
    CGRect _initialViewRect;
    CGFloat _initilalTableViewHeight;
}
@property (weak) IBOutlet NSTableView *tableView;
@property (weak) IBOutlet NSTextField *explanationLabel;
@property (weak) IBOutlet NSLayoutConstraint *tableViewHeightConstraint;
@end

@implementation SynthesizerViewController

- (NSString *)identifier
{
    return @"Synthesizer";
}

- (NSImage *)toolbarItemImage
{
    return [NSImage imageNamed:NSImageNameApplicationIcon];
}

- (NSString *)toolbarItemLabel
{
    return NSLocalizedString(@"Preference_Synthesizer", @"Synthesizer");
}

- (void)layout
{
    [self view];
    self.view.frame = self.desiredViewFrame;
    _tableViewHeightConstraint.constant = self.desiredTableViewHeight;
}

- (CGRect)desiredViewFrame
{
    CGFloat height = _initialViewRect.size.height + (_manager.descriptions.count - 1) * _initilalTableViewHeight - _explanationLabel.frame.size.height + self.desiredExplanationLabelHeight;
    return CGRectMake(0, 0, self.view.frame.size.width, height);
}

- (CGFloat)desiredTableViewHeight
{
    return _initilalTableViewHeight * _manager.descriptions.count;
}

- (CGFloat)desiredExplanationLabelHeight
{
    return 1 < _manager.descriptions.count ? _explanationLabel.frame.size.height : 0;
}

- (BOOL)isExplanationLabelHidden
{
    return 2 > _manager.descriptions.count;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _initialViewRect = self.view.frame;
    _initilalTableViewHeight = _tableView.frame.size.height;
    
    _manager = [MidiSourceManagerRepresentation sharedInstance];
    [_manager addObserver:self];
    
    _tableView.dataSource = self;
    _tableView.delegate = self;
    
    _explanationLabel.hidden = self.isExplanationLabelHidden;
    
    [_tableView reloadData];
}

- (void)dealloc
{
    [_manager removeObserver:self];
}

- (IBAction)addSynthesizerButtonPressed:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.allowedFileTypes = @[@"sf2"];
    
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        if (NSFileHandlingPanelOKButton == result) {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                [_manager loadMidiSourceDescriptionFromSoundFont:openPanel.URL.path];
            });
        }
    }];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [super mouseDown:theEvent];
    [self.view.window makeFirstResponder:self];
}

- (void)resizeWindowFrameAndReload
{
    CGRect newWindowFrame = [self.view.window frameRectForContentRect:self.desiredViewFrame];
    newWindowFrame.origin.x = NSMinX(self.view.window.frame);
    newWindowFrame.origin.y = NSMinY(self.view.window.frame) + (NSHeight(self.view.window.frame) - NSHeight(newWindowFrame));
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        context.duration = 0.25;
        context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
        [self.view.window.animator setFrame:newWindowFrame display:YES];
        _tableViewHeightConstraint.animator.constant = self.desiredTableViewHeight;
        _explanationLabel.animator.hidden = self.isExplanationLabelHidden;
    } completionHandler:^{
        [_tableView reloadData];
    }];
}

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return _manager.descriptions.count;
}

- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)row
{
    SynthesizerRowView *view = [tableView makeViewWithIdentifier:@"SynthesizerRow" owner:nil];
    view.delegate = self;
    view.description = _manager.descriptions[row];
    return view;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

#pragma mark SynthesizerRowViewDelegate

- (void)synthesizerRowViewDidClickUnload:(SynthesizerRowView *)view
{
    [_manager unloadMidiSourceDescription:view.description];
}

#pragma mark MidiSourceManagerRepresentationObserver

- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onLoadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    [self resizeWindowFrameAndReload];
}

- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onUnloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    [self resizeWindowFrameAndReload];
}

@end
