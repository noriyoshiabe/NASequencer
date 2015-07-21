#pragma once

#import <Foundation/Foundation.h>

#import "MidiEvent.h"

@interface MixerChannel : NSObject

@property (nonatomic, readonly) uint8_t channelNumber;
@property (nonatomic, readonly) Level level;

@property (nonatomic) MidiSourceRepresentation *midiSource;
@property (nonatomic) PresetRepresentation *preset;

@property (nonatomic) uint8_t volume;
@property (nonatomic) uint8_t pan;
@property (nonatomic) uint8_t chorusSend;
@property (nonatomic) uint8_t reverbSend;

@end

@interface Mixer : NSObject

@property (nonatomic, readonly) NSArray *midiSources;

+ (Mixer *)sharedInstance;
- (void)initialize;
- (void)sendNoteOn:(NoteEvent *)event;
- (void)sendNoteOff:(NoteEvent *)event;
- (void)sendAllNoteOff;
- (void)sendSound:(SoundEvent *)event;

- (PresetRepresentation *)preset:(uint8_t)channel;
- (NSArray *)presetList:(uint8_t)channel;
- (MidiSourceDescription *)midiSourceDescription:(uint8_t)channel;

@end
