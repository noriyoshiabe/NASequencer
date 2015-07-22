#pragma once

#import <Foundation/Foundation.h>

#import "MidiEvent.h"
#import "MidiSourceManager.h"

@interface MidiSourceRepresentation : NSObject

@property (nonatomic, readonly) MidiSourceDescription *description;
@property (nonatomic, readonly) NSArray *presets;
@property (nonatomic, readonly) Level level;

@end

@interface PresetRepresentation : NSObject

@property (nonatomic, readonly) NSString *name;
@property (nonatomic, readonly) uint16_t bankNo;
@property (nonatomic, readonly) uint8_t programNo;

@end

@interface ChannelRepresentation : NSObject

@property (nonatomic, readonly) uint8_t number;
@property (nonatomic, readonly) Level level;

@property (nonatomic, strong) MidiSourceRepresentation *midiSource;
@property (nonatomic, strong) PresetRepresentation *preset;

@property (nonatomic) uint8_t volume;
@property (nonatomic) uint8_t pan;
@property (nonatomic) uint8_t chorusSend;
@property (nonatomic) uint8_t reverbSend;

@property (nonatomic) bool mute;
@property (nonatomic) bool solo;

@end

@protocol MixerObserver;

@interface Mixer : NSObject

@property (nonatomic, readonly) NSArray *midiSources;
@property (nonatomic, readonly) NSArray *channels;
@property (nonatomic, readonly) Level level;

- (void)sendNoteOn:(NoteEvent *)event;
- (void)sendNoteOff:(NoteEvent *)event;
- (void)sendAllNoteOff;
- (void)sendSound:(SoundEvent *)event;

- (void)addObserver:(id<MixerObserver>)observer;
- (void)removeObserver:(id<MixerObserver>)observer;

@end

@protocol MixerObserver <NSObject>
@optional
- (void)mixer:(Mixer *)mixer onChannelChange:(ChannelRepresentation *)channel;
- (void)mixer:(Mixer *)mixer onAvailableMidisourceChange:(NSArray *)midiSources;
- (void)mixerOnLevelUpdate:(Mixer *)mixer;
@end
