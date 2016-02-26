//
//  MixerChannelViewController.m
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MixerChannelViewController.h"
#import "LevelIndicator.h"
#import "Color.h"

@interface MixerChannelViewController () <MixerRepresentationObserver> {
    MixerChannelRepresentation *_mixerChannel;
}

@property (weak) IBOutlet LevelIndicator *indicatorL;
@property (weak) IBOutlet LevelIndicator *indicatorR;
@property (weak) IBOutlet NSView *underLine;

@property (assign, nonatomic) bool mute;
@property (assign, nonatomic) bool solo;
@property (assign, nonatomic) int volume;
@property (assign, nonatomic) int pan;
@property (assign, nonatomic) int chorus;
@property (assign, nonatomic) int reverb;
@property (assign, nonatomic) int L;
@property (assign, nonatomic) int R;

@property (readonly, nonatomic) NSArray<MidiSourceDescriptionRepresentation *> *availableDescriptions;
@property (readonly, nonatomic) NSArray<PresetRepresentation *> *presets;
@property (readwrite, nonatomic) MidiSourceDescriptionRepresentation *midiSourceDescription;
@property (readwrite, nonatomic) PresetRepresentation *preset;
@end

@implementation MixerChannelViewController

- (void)setChannel:(int)channel
{
    _channel = channel;
    _mixerChannel = _mixer.channels[channel - 1];
    _L = -1440;
    _R = -1440;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    _underLine.wantsLayer = YES;
    _underLine.layer.backgroundColor = [Color gray].CGColor;
    
    [_indicatorL bind:@"intValue" toObject:self withKeyPath:@"L" options:nil];
    [_indicatorR bind:@"intValue" toObject:self withKeyPath:@"R" options:nil];
    
    [_mixer addObserver:self];
}

- (void)dealloc
{
    [_mixer removeObserver:self];
}

- (NSArray<MidiSourceDescriptionRepresentation *> *)availableDescriptions
{
    return [MidiSourceManagerRepresentation sharedInstance].availableDescriptions;
}

- (MidiSourceDescriptionRepresentation *)midiSourceDescription
{
    return _mixerChannel.midiSourceDescription;
}

- (NSArray<PresetRepresentation *> *)presets
{
    return _mixerChannel.presets;
}

- (PresetRepresentation *)preset
{
    return _mixerChannel.preset;
}

- (bool)mute
{
    return _mixerChannel.mute;
}

- (bool)solo
{
    return _mixerChannel.solo;
}

- (int)volume
{
    return _mixerChannel.volume;
}

- (int)pan
{
    return _mixerChannel.pan;
}

- (int)chorus
{
    return _mixerChannel.chorus;
}

- (int)reverb
{
    return _mixerChannel.reverb;
}

- (void)setMidiSourceDescription:(MidiSourceDescriptionRepresentation *)midiSourceDescription
{
    _mixerChannel.midiSourceDescription = midiSourceDescription;
}

- (void)setPreset:(PresetRepresentation *)preset
{
    _mixerChannel.preset = preset;
}

- (void)setMute:(bool)mute
{
    _mixerChannel.mute = mute;
}

- (void)setSolo:(bool)solo
{
    _mixerChannel.solo = solo;
}

- (void)setVolume:(int)volume
{
    _mixerChannel.volume = volume;
}

- (void)setPan:(int)pan
{
    _mixerChannel.pan = pan;
}

- (void)setChorus:(int)chorus
{
    _mixerChannel.chorus = chorus;
}

- (void)setReverb:(int)reverb
{
    _mixerChannel.reverb = reverb;
}

- (void)notifyValueChangeForKey:(NSString *)key
{
    [self willChangeValueForKey:key];
    [self didChangeValueForKey:key];
}

#pragma mark MixerRepresentationObserver

- (void)mixer:(MixerRepresentation *)mixer onChannelStatusChange:(MixerChannelRepresentation *)channel kind:(MixerChannelStatusKind)kind
{
    if (channel == _mixerChannel) {
        switch (kind) {
            case MixerChannelStatusKindMidiSourceDescription:
                [self notifyValueChangeForKey:@"midiSourceDescription"];
                [self notifyValueChangeForKey:@"presets"];
                break;
            case MixerChannelStatusKindPreset:
                [self notifyValueChangeForKey:@"preset"];
                break;
            case MixerChannelStatusKindVolume:
                [self notifyValueChangeForKey:@"volume"];
                break;
            case MixerChannelStatusKindPan:
                [self notifyValueChangeForKey:@"pan"];
                break;
            case MixerChannelStatusKindChorusSend:
                [self notifyValueChangeForKey:@"chorus"];
                break;
            case MixerChannelStatusKindReverbSend:
                [self notifyValueChangeForKey:@"reverb"];
                break;
            case MixerChannelStatusKindExpressionSend:
                break;
            case MixerChannelStatusKindMute:
                [self notifyValueChangeForKey:@"mute"];
                break;
            case MixerChannelStatusKindSolo:
                [self notifyValueChangeForKey:@"solo"];
                break;
        }
    }
}

- (void)mixer:(MixerRepresentation *)mixer onAvailableMidiSourceChange:(NSArray<MidiSourceDescriptionRepresentation *> *)descriptions
{
    [self notifyValueChangeForKey:@"availableDescriptions"];
}

- (void)mixerOnLevelUpdate:(MixerRepresentation *)mixer
{
    self.L = _mixerChannel.level.L;
    self.R = _mixerChannel.level.R;
}

@end

#pragma mark For Binding

@interface PresetRepresentation (MixerChannelView)
- (NSString *)selectionLabel;
@end

@implementation PresetRepresentation (MixerChannelView)

- (NSString *)selectionLabel
{
    return [NSString stringWithFormat:@"%@ bank:%d prg:%d", self.name, self.bankNo, self.programNo];
}

@end
