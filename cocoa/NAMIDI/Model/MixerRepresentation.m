//
//  MixerRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/26/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MixerRepresentation.h"

@interface PresetRepresentation ()
- (instancetype)initWithPresetInfo:(PresetInfo *)presetInfo;
@property (readonly, nonatomic) PresetInfo *raw;
@end

@implementation PresetRepresentation

- (instancetype)initWithPresetInfo:(PresetInfo *)presetInfo
{
    self = [super init];
    if (self) {
        _raw = presetInfo;
    }
    return self;
}

- (NSString *)name
{
    return [NSString stringWithUTF8String:_raw->name];
}

- (uint16_t)bankNo
{
    return _raw->bankNo;
}

- (uint8_t)programNo
{
    return _raw->programNo;
}

@end

@interface MixerChannelRepresentation () {
    MixerChannel *_raw;
    MidiSourceDescriptionRepresentation *_midiSourceDescription;
    NSMutableArray<PresetRepresentation *> *_presets;
    PresetRepresentation *_preset;
}

- (instancetype)initWithMixerChannel:(MixerChannel *)channel;
- (void)loadMidiSourceDescription;
- (void)loadPreset;
@end

@implementation MixerChannelRepresentation

- (instancetype)initWithMixerChannel:(MixerChannel *)channel
{
    self = [super init];
    if (self) {
        _raw = channel;
        [self loadMidiSourceDescription];
        [self loadPreset];
    }
    return self;
}

- (void)loadMidiSourceDescription
{
    MidiSourceDescription *description = MixerChannelGetMidiSourceDescription(_raw);
    _midiSourceDescription = [[MidiSourceManagerRepresentation sharedInstance].availableDescriptions objectPassingTest:^BOOL(MidiSourceDescriptionRepresentation *obj, NSUInteger idx, BOOL *stop) {
        return *stop = obj.raw == description;
    }];
    
    _presets = [NSMutableArray array];
    
    int count = MixerChannelGetPresetCount(_raw);
    PresetInfo **infos = MixerChannelGetPresetInfos(_raw);
    for (int i = 0; i < count; ++i) {
        PresetRepresentation *preset = [[PresetRepresentation alloc] initWithPresetInfo:infos[i]];
        [_presets addObject:preset];
    }
}

- (void)loadPreset
{
    PresetInfo *info = MixerChannelGetPresetInfo(_raw);
    _preset = [_presets objectPassingTest:^BOOL(PresetRepresentation *obj, NSUInteger idx, BOOL *stop) {
        return *stop = obj.raw == info;
    }];
}

- (int)number
{
    return MixerChannelGetNumber(_raw);
}

- (MidiSourceDescriptionRepresentation *)midiSourceDescription
{
    return _midiSourceDescription;
}

- (PresetRepresentation *)preset
{
    return _preset;
}

- (Level)level
{
    return MixerChannelGetLevel(_raw);
}

- (int)volume
{
    return MixerChannelGetVolume(_raw);
}

- (int)pan
{
    return MixerChannelGetPan(_raw);
}

- (int)chorus
{
    return MixerChannelGetChorusSend(_raw);
}

- (int)reverb
{
    return MixerChannelGetReverbSend(_raw);
}

- (bool)mute
{
    return MixerChannelGetMute(_raw);
}

- (bool)solo
{
    return MixerChannelGetSolo(_raw);
}

- (void)setMidiSourceDescription:(MidiSourceDescriptionRepresentation *)midiSourceDescription
{
    MixerChannelSetMidiSourceDescription(_raw, midiSourceDescription.raw);
}

- (void)setPreset:(PresetRepresentation *)preset
{
    MixerChannelSetPresetInfo(_raw, preset.raw);
}

- (void)setVolume:(int)volume
{
    MixerChannelSetVolume(_raw, volume);
}

- (void)setPan:(int)pan
{
    MixerChannelSetPan(_raw, pan);
}

- (void)setChorus:(int)chorus
{
    MixerChannelSetChorusSend(_raw, chorus);
}

- (void)setReverb:(int)reverb
{
    MixerChannelSetReverbSend(_raw, reverb);
}

- (void)setMute:(bool)mute
{
    MixerChannelSetMute(_raw, mute);
}

- (void)setSolo:(bool)solo
{
    MixerChannelSetSolo(_raw, solo);
}

@end

@interface MixerRepresentation () {
    Mixer *_mixer;
    NSHashTable *_observers;
    NSMutableArray<MixerChannelRepresentation *> *_channels;
}
- (void)onChannelStatusChange:(MixerChannel *)channel kind:(MixerChannelStatusKind)kind;
- (void)onAvailableMidiSourceChange:(NAArray *)descriptions;
- (void)onLevelUpdate;
@end

static void onChannelStatusChange(void *receiver, MixerChannel *channel, MixerChannelStatusKind kind)
{
    MixerRepresentation *mixer = (__bridge MixerRepresentation *)receiver;
    [mixer onChannelStatusChange:channel kind:kind];
}

static void onAvailableMidiSourceChange(void *receiver, NAArray *descriptions)
{
    MixerRepresentation *mixer = (__bridge MixerRepresentation *)receiver;
    [mixer onAvailableMidiSourceChange:descriptions];
}

static void onLevelUpdate(void *receiver)
{
    MixerRepresentation *mixer = (__bridge MixerRepresentation *)receiver;
    [mixer onLevelUpdate];
}

static MixerObserverCallbacks callbacks = {onChannelStatusChange, onAvailableMidiSourceChange, onLevelUpdate};

@implementation MixerRepresentation

- (instancetype)initWithMixer:(Mixer *)mixer
{
    self = [super init];
    if (self) {
        _mixer = mixer;
        _observers = [NSHashTable weakObjectsHashTable];
        _channels = [NSMutableArray array];
        
        NAArray *channels = MixerGetChannels(_mixer);
        NAIterator *iterator = NAArrayGetIterator(channels);
        while (iterator->hasNext(iterator)) {
            [_channels addObject:[[MixerChannelRepresentation alloc] initWithMixerChannel:iterator->next(iterator)]];
        }
        
        MixerAddObserver(_mixer, (__bridge void *)self, &callbacks);
    }
    return self;
    
}

- (void)finalize
{
    if (_mixer) {
        MixerRemoveObserver(_mixer, (__bridge void *)self);
    }
}

- (void)addObserver:(id<MixerRepresentationObserver>)observer
{
    [_observers addObject:observer];
}

- (void)removeObserver:(id<MixerRepresentationObserver>)observer
{
    [_observers removeObject:observer];
}

- (Level)level
{
    return MixerGetLevel(_mixer);
}

- (void)onChannelStatusChange:(MixerChannel *)channel kind:(MixerChannelStatusKind)kind
{
    MixerChannelRepresentation *__channel = _channels[MixerChannelGetNumber(channel) - 1];
    
    switch (kind) {
        case MixerChannelStatusKindMidiSourceDescription:
            [__channel loadMidiSourceDescription];
            break;
        case MixerChannelStatusKindPreset:
            [__channel loadPreset];
            break;
        default:
            break;
    }
    
    [NSThread performBlockOnMainThread:^{
        for (id<MixerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(mixer:onChannelStatusChange:kind:)]) {
                [observer mixer:self onChannelStatusChange:__channel kind:kind];
            }
        }
    }];
}

- (void)onAvailableMidiSourceChange:(NAArray *)descriptions
{
    NSArray<MidiSourceDescriptionRepresentation *> *availableDescriptions = [MidiSourceManagerRepresentation sharedInstance].availableDescriptions;
    
    [NSThread performBlockOnMainThread:^{
        for (id<MixerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(mixer:onAvailableMidiSourceChange:)]) {
                [observer mixer:self onAvailableMidiSourceChange:availableDescriptions];
            }
        }
    }];
}

- (void)onLevelUpdate
{
    [NSThread performBlockOnMainThread:^{
        for (id<MixerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(mixerOnLevelUpdate:)]) {
                [observer mixerOnLevelUpdate:self];
            }
        }
    }];
}

@end
