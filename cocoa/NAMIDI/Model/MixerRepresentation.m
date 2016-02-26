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
    MidiSourceDescriptionRepresentation *_description;
    NSMutableArray<PresetRepresentation *> *_presets;
    PresetRepresentation *_preset;
}

- (instancetype)initWithMixerChannel:(MixerChannel *)channel;
- (void)reload;
@end

@implementation MixerChannelRepresentation

- (instancetype)initWithMixerChannel:(MixerChannel *)channel
{
    self = [super init];
    if (self) {
        _raw = channel;
        _presets = [NSMutableArray array];
        [self loadCache];
    }
    return self;
}

- (void)loadCache
{
    _description = [[MidiSourceManagerRepresentation sharedInstance].availableDescriptions objectPassingTest:^BOOL(MidiSourceDescriptionRepresentation *obj, NSUInteger idx, BOOL *stop) {
        return *stop = 0 == strcmp(obj.raw->name, MixerChannelGetMidiSourceDescription(_raw)->name);
    }];
    
    PresetInfo *info = MixerChannelGetPresetInfo(_raw);
    int count = MixerChannelGetPresetCount(_raw);
    PresetInfo **infos = MixerChannelGetPresetInfos(_raw);
    for (int i = 0; i < count; ++i) {
        PresetRepresentation *preset = [[PresetRepresentation alloc] initWithPresetInfo:infos[i]];
        [_presets addObject:preset];
        if (info == infos[i]) {
            _preset = preset;
        }
    }
}

- (void)reload
{
    [_presets removeAllObjects];
    [self loadCache];
}

- (int)number
{
    return MixerChannelGetNumber(_raw);
}

- (MidiSourceDescriptionRepresentation *)description
{
    return _description;
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

- (void)setPreset:(PresetRepresentation *)preset
{
    MixerChannelSetPresetInfo(_raw, _preset.raw);
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
- (void)onChannelStatusChange:(MixerChannel *)channel;
- (void)onAvailableMidiSourceChange:(NAArray *)descriptions;
- (void)onLevelUpdate;
@end

static void onChannelStatusChange(void *receiver, MixerChannel *channel)
{
    MixerRepresentation *mixer = (__bridge MixerRepresentation *)receiver;
    [mixer onChannelStatusChange:channel];
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
    }
    return self;
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

- (void)onChannelStatusChange:(MixerChannel *)channel
{
    MixerChannelRepresentation *__channel = _channels[MixerChannelGetNumber(channel) - 1];
    
    [NSThread performBlockOnMainThread:^{
        for (id<MixerRepresentationObserver> observer in _observers) {
            [observer mixer:self onChannelStatusChange:__channel];
        }
    }];
}

- (void)onAvailableMidiSourceChange:(NAArray *)descriptions
{
    NSArray<MidiSourceDescriptionRepresentation *> *availableDescriptions = [MidiSourceManagerRepresentation sharedInstance].availableDescriptions;
    
    [NSThread performBlockOnMainThread:^{
        for (id<MixerRepresentationObserver> observer in _observers) {
            [observer mixer:self onAvailableMidiSourceChange:availableDescriptions];
        }
    }];
}

- (void)onLevelUpdate
{
    [NSThread performBlockOnMainThread:^{
        for (id<MixerRepresentationObserver> observer in _observers) {
            [observer mixerOnLevelUpdate:self];
        }
    }];
}

@end
