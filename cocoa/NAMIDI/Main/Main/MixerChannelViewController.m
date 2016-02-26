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

@interface MixerChannelViewController () {
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
@property (readonly, nonatomic) MidiSourceDescriptionRepresentation *description;
@property (readonly, nonatomic) NSArray<PresetRepresentation *> *presets;
@property (readonly, nonatomic) PresetRepresentation *preset;
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
    
    [_indicatorL bind:@"intValue" toObject:self withKeyPath:@"L" options:nil];
    [_indicatorR bind:@"intValue" toObject:self withKeyPath:@"R" options:nil];
}

- (NSArray<MidiSourceDescriptionRepresentation *> *)availableDescriptions
{
    return [MidiSourceManagerRepresentation sharedInstance].availableDescriptions;
}

- (MidiSourceDescriptionRepresentation *)description
{
    return _mixerChannel.description;
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

- (void)setMute:(bool)mute
{
    NSLog(@"%s %d", __func__, mute);
    _mute = mute;
}

- (void)setSolo:(bool)solo
{
    NSLog(@"%s %d", __func__, solo);
    _solo = solo;
}

- (void)setVolume:(int)volume
{
    NSLog(@"%s %d", __func__, volume);
    _volume = volume;
}

- (void)setPan:(int)pan
{
    NSLog(@"%s %d", __func__, pan);
    _pan = pan;
}

- (void)setChorus:(int)chorus
{
    NSLog(@"%s %d", __func__, chorus);
    _chorus = chorus;
}

- (void)setReverb:(int)reverb
{
    NSLog(@"%s %d", __func__, reverb);
    _reverb = reverb;
}

- (void)setMidiSource:(MidiSourceRepresentation *)midiSource
{
    NSLog(@"%s %@", __func__, midiSource.name);
    _midiSource = midiSource;
}

- (void)setPreset:(PresetRepresentation *)preset
{
    NSLog(@"%s %@", __func__, preset.name);
    _preset = preset;
}

@end
