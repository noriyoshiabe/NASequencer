#pragma once

#import <Foundation/Foundation.h>

#import "MidiClient.h"

///////// another file

@interface PresetRepresentation : NSObject
@property (nonatomic, readonly) NSString *name;
@property (nonatomic, readonly) uint16_t bankNo;
@property (nonatomic, readonly) uint8_t programNo;
// PresetList *preset;
@end

@interface MidiSourceRepresentation : NSObject
@property (nonatomic, readonly) MidiSourceDescription *description;

@property (nonatomic, readonly) NSArray *presets;
@property (nonatomic, readonly) Level level;

- (PresetRepresentation *)preset:(uint8_t)channel;
- (void)setPreset:(PresetRepresentation *)preset channel:(uint8_t)channel;

- (Level)level:(uint8_t)channel;

- (uint8_t)volume:(uint8_t)channel;
- (int8_t)pan:(uint8_t)channel;
- (uint8_t)chorusSend:(uint8_t)channel;
- (uint8_t)reverbSend:(uint8_t)channel;

- (void)setVolume:(uint8_t)volume channel:(uint8_t)channel;
- (void)setPan:(int8_t)pan channel:(uint8_t)channel;
- (void)setChorusSend:(uint8_t)chorusSend channel:(uint8_t)channel;
- (void)setReverbSend:(uint8_t)reverbSend channel:(uint8_t)channel;

- (void)send:(uint8_t *)bytes length:(size_t)length;
- (void)computeAudioSample:(AudioSample *)buffer count:(uint32_t)count;

// TODO Callback
// MidiSourceDescription *description;
// MidiSource *midiSource;

@end

@interface MidiSourceDescription : NSObject
@property (nonatomic, readonly) NSString *name;
- (MidiSourceRepresentation *)createMidiSourceRepresentation;
@end

@interface MidiSourceManager : NSObject

@property (nonatomic, readonly) NSArray *descriptions;

+ (MidiSourceManager *)sharedInstance;
- (void)loadSoundFont:(NSString *)filepath;
- (void)unloadMidiSource:(MidiSourceDescription *)description;

@end
