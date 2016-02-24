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
#import "Stub.h"

@interface MixerChannelViewController () {
    NSMutableArray *_midiSources;
    NSArray *_presets;
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

@property (strong, nonatomic) MidiSourceRepresentation *midiSource;
@property (strong, nonatomic) PresetRepresentation *preset;
@end

@implementation MixerChannelViewController

- (instancetype)init
{
    self = [super init];
    if (self) {
        // TODO
        
        _midiSources = [@[
                         [[MidiSourceRepresentation alloc] init],
                         [[MidiSourceRepresentation alloc] init],
                         [[MidiSourceRepresentation alloc] init],
                         ] mutableCopy];
        
        _midiSource = _midiSources.firstObject;
        
        _presets = @[
                     [[PresetRepresentation alloc] init],
                     [[PresetRepresentation alloc] init],
                     [[PresetRepresentation alloc] init],
                     ];
        
        _preset = _presets.firstObject;
        
        _L = -300;
        _R = -300;
        
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    _underLine.wantsLayer = YES;
    _underLine.layer.backgroundColor = [Color gray].CGColor;
    
    [_indicatorL bind:@"intValue" toObject:self withKeyPath:@"L" options:nil];
    [_indicatorR bind:@"intValue" toObject:self withKeyPath:@"R" options:nil];
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
