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

@property (weak) IBOutlet NSButton *muteButton;
@property (weak) IBOutlet NSButton *soloButton;
@property (weak) IBOutlet NSPopUpButton *synthButton;
@property (weak) IBOutlet NSButton *presetButton;
@property (weak) IBOutlet NSSlider *volumeSlider;
@property (weak) IBOutlet NSSlider *panSlider;
@property (weak) IBOutlet NSSlider *chorusSlider;
@property (weak) IBOutlet NSSlider *reverbSlider;
@property (weak) IBOutlet LevelIndicator *indicatorL;
@property (weak) IBOutlet LevelIndicator *indicatorR;
@property (weak) IBOutlet NSView *underLine;

@property (assign, nonatomic) bool mute;
@property (assign, nonatomic) bool solo;
@property (assign, nonatomic) int volume;
@property (assign, nonatomic) int pan;
@property (assign, nonatomic) int chorus;
@property (assign, nonatomic) int reverb;

@property (readonly, nonatomic) int L;
@property (readonly, nonatomic) int R;

@property (readonly, nonatomic) NSArray<MidiSourceDescriptionRepresentation *> *availableDescriptions;
@property (readwrite, nonatomic) MidiSourceDescriptionRepresentation *midiSourceDescription;
@property (readonly, nonatomic) NSString *presetLabel;
@end

@implementation MixerChannelViewController

- (void)setChannel:(int)channel
{
    _channel = channel;
    _mixerChannel = _mixer.channels[channel - 1];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    _underLine.wantsLayer = YES;
    _underLine.layer.backgroundColor = [Color gray].CGColor;
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    [_muteButton bind:@"value" toObject:self withKeyPath:@"mute" options:nil];
    [_soloButton bind:@"value" toObject:self withKeyPath:@"solo" options:nil];
    
    [_synthButton bind:@"content" toObject:self withKeyPath:@"availableDescriptions" options:nil];
    [_synthButton bind:@"contentValues" toObject:self withKeyPath:@"availableDescriptions.name" options:nil];
    [_synthButton bind:@"selectedObject" toObject:self withKeyPath:@"midiSourceDescription" options:nil];
    
    [_presetButton bind:@"title" toObject:self withKeyPath:@"presetLabel" options:nil];
    
    [_volumeSlider bind:@"value" toObject:self withKeyPath:@"volume" options:nil];
    [_panSlider bind:@"value" toObject:self withKeyPath:@"pan" options:nil];
    [_chorusSlider bind:@"value" toObject:self withKeyPath:@"chorus" options:nil];
    [_reverbSlider bind:@"value" toObject:self withKeyPath:@"reverb" options:nil];
    
    [_indicatorL bind:@"intValue" toObject:self withKeyPath:@"L" options:nil];
    [_indicatorR bind:@"intValue" toObject:self withKeyPath:@"R" options:nil];
    
    [_mixer addObserver:self];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    
    [_muteButton unbind:@"value"];
    [_soloButton unbind:@"value"];
    
    [_synthButton unbind:@"contentValues"];
    [_synthButton unbind:@"content"];
    [_synthButton unbind:@"selectedObject"];
    
    [_presetButton unbind:@"title"];
    
    [_volumeSlider unbind:@"value"];
    [_panSlider unbind:@"value"];
    [_chorusSlider unbind:@"value"];
    [_reverbSlider unbind:@"value"];
    
    [_indicatorL unbind:@"intValue"];
    [_indicatorR unbind:@"intValue"];
    
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

- (NSString *)presetLabel
{
    return _mixerChannel.preset.name;
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

- (int)L
{
    return _mixerChannel.level.L;
}

- (int)R
{
    return _mixerChannel.level.R;
}

- (void)setMidiSourceDescription:(MidiSourceDescriptionRepresentation *)midiSourceDescription
{
    _mixerChannel.midiSourceDescription = midiSourceDescription;
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

- (IBAction)presetButtonPressed:(id)sender
{
    [_delegate mixerChannelViewController:self didSelectPresetButtonWithChannel:_mixerChannel];
}

#pragma mark MixerRepresentationObserver

- (void)mixer:(MixerRepresentation *)mixer onChannelStatusChange:(MixerChannelRepresentation *)channel kind:(MixerChannelStatusKind)kind
{
    if (channel == _mixerChannel) {
        switch (kind) {
            case MixerChannelStatusKindMidiSourceDescription:
                [self notifyValueChangeForKey:@"midiSourceDescription"];
                [self notifyValueChangeForKey:@"presetLabel"];
                break;
            case MixerChannelStatusKindPreset:
                [self notifyValueChangeForKey:@"presetLabel"];
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
    [self notifyValueChangeForKey:@"L"];
    [self notifyValueChangeForKey:@"R"];
}

@end
