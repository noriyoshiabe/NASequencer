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

@interface SynthesizerViewController () <NSTableViewDataSource, NSTableViewDelegate, MidiSourceManagerRepresentationObserver, SynthesizerRowViewDelegate> {
    MidiSourceManagerRepresentation *_manager;
}
@property (weak) IBOutlet NSTableView *tableView;
@property (weak) IBOutlet NSTextField *explanationLabel;
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

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _manager = [MidiSourceManagerRepresentation sharedInstance];
    [_manager addObserver:self];
    
    _tableView.dataSource = self;
    _tableView.delegate = self;
    
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
    [_tableView reloadData];
}

- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onUnloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    [_tableView reloadData];
}

@end
