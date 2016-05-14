//
//  PresetSelectionWindowController.m
//  NAMIDI
//
//  Created by abechan on 4/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PresetSelectionWindowController.h"
#import "ColorButton.h"

@interface PresetKeyboardButton : ColorButton
@property (weak, nonatomic) IBOutlet PresetSelectionWindowController *controller;
@property (assign, nonatomic) unichar key;
@property (assign, nonatomic) int sentNoteNo;
- (void)sendNoteOn:(unichar)key;
- (void)sendNoteOff;
- (void)reset;
@end

@interface PresetSelectionWindowController () <NSWindowDelegate, NSTableViewDataSource, NSTableViewDelegate, MixerRepresentationObserver, NSWindowDelegate> {
    int _octave;
    id _keyDownMonitor;
}
@property (weak) IBOutlet NSTextField *channelField;
@property (weak) IBOutlet NSTextField *synthesizerNameField;
@property (weak) IBOutlet NSTextField *bankNoField;
@property (weak) IBOutlet NSTextField *programNoField;
@property (weak) IBOutlet NSTableView *presetTableView;
@property (strong, nonatomic) NSMutableDictionary<NSNumber *, PresetKeyboardButton *> *keys;
@property (weak) IBOutlet NSTextField *octaveLabel1;
@property (weak) IBOutlet NSTextField *octaveLabel2;
@end

@implementation PresetSelectionWindowController

- (instancetype)init
{
    self = [super init];
    if (self) {
        _keys = [NSMutableDictionary dictionary];
    }
    return self;
}

- (NSString *)windowNibName
{
    return @"PresetSelectionWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    self.window.preventsApplicationTerminationWhenModal = NO;
    
    _octave = 2;
    [self updateOctaveLabel];
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
        
        [self cancelAllKeys];
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

- (IBAction)performClose:(id)sender
{
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseCancel];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    [_presetTableView.nextResponder becomeFirstResponder];
    
    _keyDownMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask | NSKeyUpMask | NSFlagsChangedMask handler:self.keyDownMonitorHandler];
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    [NSEvent removeMonitor:_keyDownMonitor];
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

- (void)cancelAllKeys
{
    for (PresetKeyboardButton *button in _keys.allValues) {
        [button reset];
    }
}

- (void)updateOctaveLabel
{
    for (PresetKeyboardButton *button in _keys.allValues) {
        if (-1 == [self noteNo:button.key]) {
            button.enabled = NO;
        }
        else {
            button.enabled = YES;
        }
    }
    
    _octaveLabel1.stringValue = [NSString stringWithFormat:@"C%d", _octave];
    if (8 > _octave) {
        _octaveLabel2.stringValue = [NSString stringWithFormat:@"C%d", _octave + 1];
    }
    else {
        _octaveLabel2.stringValue = @"";
    }
}

- (BOOL)handleKeyDown:(NSEvent *)event
{
    unichar key = [event.charactersIgnoringModifiers characterAtIndex:0];
    switch (key) {
        case NSCarriageReturnCharacter:
            if (!event.isARepeat) {
                [self okButtonPressed:self];
            }
            return YES;
            
        case NSLeftArrowFunctionKey:
            if (event.modifierFlags & NSShiftKeyMask) {
                return NO;
            }
            else {
                if (-2 < _octave) {
                    --_octave;
                    [self updateOctaveLabel];
                }
                return YES;
            }
            
        case NSRightArrowFunctionKey:
            if (event.modifierFlags & NSShiftKeyMask) {
                return NO;
            }
            else {
                if (8 > _octave) {
                    ++_octave;
                    [self updateOctaveLabel];
                }
                return YES;
            }
            
        case NSUpArrowFunctionKey:
        case NSDownArrowFunctionKey:
            [_presetTableView keyDown:event];
            return YES;
            
        default:
            if (!event.isARepeat && !(event.modifierFlags & NSCommandKeyMask)) {
                PresetKeyboardButton *button = _keys[@(key)];
                if (button.enabled) {
                    button.state = NSOnState;
                    [button sendNoteOn:key];
                }
            }
            
            return YES;
    }
}

- (BOOL)handleKeyUp:(NSEvent *)event
{
    unichar key = [event.charactersIgnoringModifiers characterAtIndex:0];
    if (_keys[@(key)]) {
        _keys[@(key)].state = NSOffState;
        [_keys[@(key)] sendNoteOff];
        return YES;
    }
    else {
        return NO;
    }
}

- (BOOL)handleFlagsChanged:(NSEvent *)event
{
    if (event.modifierFlags & NSCommandKeyMask) {
        [self cancelAllKeys];
        return YES;
    }
    else {
        return NO;
    }
}

- (NSEvent* __nullable (^)(NSEvent*))keyDownMonitorHandler
{
    return ^NSEvent * _Nullable(NSEvent *event) {
        if (53 == event.keyCode) {
            [self cancelOperation:self];
            return nil;
        }
        
        switch (event.type) {
            case NSKeyDown:
                if ([self handleKeyDown:event]) {
                    return nil;
                }
                else {
                    return event;
                }
            case NSKeyUp:
                if ([self handleKeyUp:event]) {
                    return nil;
                }
                else {
                    return event;
                }
            case NSFlagsChanged:
                if ([self handleFlagsChanged:event]) {
                    return nil;
                }
                else {
                    return event;
                }
            default:
                return event;
        }
    };
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


@implementation PresetKeyboardButton

- (void)awakeFromNib
{
    [super awakeFromNib];
    [self addTrackingRect:self.bounds owner:self userData:nil assumeInside:NO];
    [_controller.keys setObject:self forKey:@(self.key)];
    _sentNoteNo = -1;
}

- (unichar)key
{
    return [self.title.lowercaseString characterAtIndex:0];
}

- (void)sendNoteOn:(unichar)key
{
    int noteNo = [_controller noteNo:key];
    if (-1 != noteNo) {
        NoteEvent note;
        note.channel = _controller.mixerChannel.number;
        note.noteNo = noteNo;
        note.velocity = 100;
        [_controller.mixer sendNoteOn:&note];
    }
    
    _sentNoteNo = noteNo;
}

- (void)sendNoteOff
{
    if (-1 != _sentNoteNo) {
        NoteEvent note;
        note.channel = _controller.mixerChannel.number;
        note.noteNo = _sentNoteNo;
        [_controller.mixer sendNoteOff:&note];
    }
}

- (void)reset
{
    [self sendNoteOff];
    _sentNoteNo = -1;
    self.state = NSOffState;
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if (!self.enabled) {
        return;
    }
    
    self.state = NSOnState;
    [self sendNoteOn:self.key];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if (!self.enabled) {
        return;
    }
    
    self.state = NSOffState;
    [self sendNoteOff];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if (!self.enabled) {
        return;
    }
    
    self.state = NSOffState;
    [self sendNoteOff];
}

- (void)setEnabled:(BOOL)enabled
{
    if (!enabled && NSOnState == self.state) {
        self.state = NSOffState;
        [self sendNoteOff];
    }
    [super setEnabled:enabled];
}

@end