//
//  PresetSelectionWindowController.m
//  NAMIDI
//
//  Created by abechan on 4/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PresetSelectionWindowController.h"

@interface PresetSelectionWindowController () <NSWindowDelegate, NSTableViewDataSource, NSTableViewDelegate, MixerRepresentationObserver>
@property (weak) IBOutlet NSTextField *channelField;
@property (weak) IBOutlet NSTextField *synthesizerNameField;
@property (weak) IBOutlet NSTextField *bankNoField;
@property (weak) IBOutlet NSTextField *programNoField;
@property (weak) IBOutlet NSTableView *presetTableView;
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

- (void)setMixerChannel:(MixerChannelRepresentation *)mixerChannel
{
    if (_mixerChannel) {
        [_synthesizerNameField unbind:@"value"];
        [_bankNoField unbind:@"value"];
        [_programNoField unbind:@"value"];
        
        _presetTableView.dataSource = nil;
        _presetTableView.delegate = nil;
        
        [_mixer removeObserver:self];
    }
    
    _mixerChannel = mixerChannel;
    
    if (_mixerChannel) {
        // Load window for first time
        [self window];
        
        _channelField.stringValue = [NSString stringWithFormat:@"Channel %d", _mixerChannel.number];
        
        [_synthesizerNameField bind:@"value" toObject:self withKeyPath:@"synthesizerName" options:nil];
        [_bankNoField bind:@"value" toObject:self withKeyPath:@"bankNo" options:nil];
        [_programNoField bind:@"value" toObject:self withKeyPath:@"programNo" options:nil];
        
        _presetTableView.dataSource = self;
        _presetTableView.delegate = self;
        
        [_presetTableView reloadData];
        [self updateSelectedPreset];
        
        [_mixer addObserver:self];
    }
}

- (void)updateSelectedPreset
{
    NSUInteger index = [_mixerChannel.presets indexOfObject:_mixerChannel.preset];
    [_presetTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:index] byExtendingSelection:NO];
    [_presetTableView scrollRowToVisible:index];
}

- (NSString *)synthesizerName
{
    return _mixerChannel.midiSourceDescription.name;
}

- (int)bankNo
{
    return _mixerChannel.preset.bankNo;
}

- (int)programNo
{
    return _mixerChannel.preset.programNo;
}

- (IBAction)okButtonPressed:(id)sender
{
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseCancel];
}

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return _mixerChannel.presets.count;
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSTableCellView *view = [tableView makeViewWithIdentifier:@"CellView" owner:nil];
    view.textField.stringValue = _mixerChannel.presets[row].name;
    return view;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    PresetRepresentation *preset = [_mixerChannel.presets objectAtIndex:_presetTableView.selectedRow];
    if (_mixerChannel.preset != preset) {
        [_mixer sendAllSoundOff:_mixerChannel];
    }
    _mixerChannel.preset = preset;
}

#pragma mark MixerRepresentationObserver

- (void)mixer:(MixerRepresentation *)mixer onChannelStatusChange:(MixerChannelRepresentation *)channel kind:(MixerChannelStatusKind)kind
{
    if (channel == _mixerChannel) {
        switch (kind) {
            case MixerChannelStatusKindMidiSourceDescription:
                [self notifyValueChangeForKey:@"synthesizerName"];
                [_presetTableView reloadData];
                [self updateSelectedPreset];
                break;
            case MixerChannelStatusKindPreset:
                [self notifyValueChangeForKey:@"bankNo"];
                [self notifyValueChangeForKey:@"programNo"];
                break;
            case MixerChannelStatusKindVolume:
            case MixerChannelStatusKindPan:
            case MixerChannelStatusKindChorusSend:
            case MixerChannelStatusKindReverbSend:
            case MixerChannelStatusKindExpressionSend:
            case MixerChannelStatusKindMute:
            case MixerChannelStatusKindSolo:
                break;
        }
    }
}

@end
