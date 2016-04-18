//
//  PresetSelectionWindowController.m
//  NAMIDI
//
//  Created by abechan on 4/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PresetSelectionWindowController.h"

@interface PresetTableView : NSTableView
@property (weak, nonatomic) PresetSelectionWindowController *controller;
@end

@interface PresetSelectionWindowController () <NSWindowDelegate, NSTableViewDataSource, NSTableViewDelegate, MixerRepresentationObserver, NSWindowDelegate> {
    int _octave;
}
@property (weak) IBOutlet NSTextField *channelField;
@property (weak) IBOutlet NSTextField *synthesizerNameField;
@property (weak) IBOutlet NSTextField *bankNoField;
@property (weak) IBOutlet NSTextField *programNoField;
@property (weak) IBOutlet PresetTableView *presetTableView;
@end

@implementation PresetSelectionWindowController

- (NSString *)windowNibName
{
    return @"PresetSelectionWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    _presetTableView.controller = self;
    _octave = 2;
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

- (void)updateOctaveLabel
{
    // TODO
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

- (void)cancelOperation:(id)sender
{
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseCancel];
}

- (int)noteNo:(unichar)character
{
    const struct {
        unichar character;
        int baseNote;
    } table[] = {
        {'a', -4},
        {'z', -3},
        {'s', -2},
        {'x', -1},
        {'c', 0},
        {'f', 1},
        {'v', 2},
        {'g', 3},
        {'b', 4},
        {'n', 5},
        {'j', 6},
        {'m', 7},
        {'k', 8},
        {',', 9},
        {'l', 10},
        {'.', 11},
        {'/', 12},
        
        {'1', 8},
        {'q', 9},
        {'2', 10},
        {'w', 11},
        {'e', 12},
        {'4', 13},
        {'r', 14},
        {'5', 15},
        {'t', 16},
        {'y', 17},
        {'7', 18},
        {'u', 19},
        {'8', 20},
        {'i', 21},
        {'9', 22},
        {'o', 23},
        {'p', 24},
    };
    
    for (int i = 0; i < sizeof(table)/sizeof(table[0]); ++i) {
        if (table[i].character == character) {
            int noteNo = table[i].baseNote + (_octave + 2) * 12;
            if (0 <= noteNo && noteNo <= 127) {
                return noteNo;
            }
            else {
                return -1;
            }
        }
    }
    
    return -1;
}

- (void)keyDown:(NSEvent *)theEvent
{
    unichar key = [theEvent.charactersIgnoringModifiers characterAtIndex:0];
    switch (key) {
        case NSCarriageReturnCharacter:
            [self okButtonPressed:self];
            break;
        case NSLeftArrowFunctionKey:
            if (-2 < _octave) {
                --_octave;
                [self updateOctaveLabel];
            }
            break;
        case NSRightArrowFunctionKey:
            if (8 > _octave) {
                ++_octave;
                [self updateOctaveLabel];
            }
            break;
        default:
        {
            if (!theEvent.isARepeat) {
                int noteNo = [self noteNo:key];
                if (-1 != noteNo) {
                    NoteEvent note;
                    note.channel = _mixerChannel.number;
                    note.noteNo = noteNo;
                    note.velocity = 100;
                    [_mixer sendNoteOn:&note];
                }
            }
        }
            break;
    }
}

- (void)keyUp:(NSEvent *)theEvent
{
    unichar key = [theEvent.charactersIgnoringModifiers characterAtIndex:0];
    int noteNo = [self noteNo:key];
    if (-1 != noteNo) {
        NoteEvent note;
        note.channel = _mixerChannel.number;
        note.noteNo = noteNo;
        [_mixer sendNoteOff:&note];
    }
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



@implementation PresetTableView

- (void)keyDown:(NSEvent *)theEvent
{
    // ESC
    if (53 == theEvent.keyCode) {
        [_controller cancelOperation:self];
        return;
    }
    
    unichar key = [theEvent.charactersIgnoringModifiers characterAtIndex:0];
    if (NSUpArrowFunctionKey == key || NSDownArrowFunctionKey == key) {
        [super keyDown:theEvent];
    }
    else {
        [_controller keyDown:theEvent];
    }
}

@end
