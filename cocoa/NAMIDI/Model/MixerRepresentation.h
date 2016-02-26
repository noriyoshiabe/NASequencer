//
//  MixerRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/26/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MidiSourceManagerRepresentation.h"

@interface PresetRepresentation : NSObject
@property (readonly, nonatomic) NSString *name;
@property (readonly, nonatomic) uint16_t bankNo;
@property (readonly, nonatomic) uint8_t programNo;
@end

@interface MixerChannelRepresentation : NSObject
@property (readonly, nonatomic) int number;
@property (readonly, nonatomic) MidiSourceDescriptionRepresentation *description;
@property (readonly, nonatomic) NSArray<PresetRepresentation *> *presets;
@property (readonly, nonatomic) Level level;
@property (strong, nonatomic) PresetRepresentation *preset;
@property (assign, nonatomic) int volume;
@property (assign, nonatomic) int pan;
@property (assign, nonatomic) int chorus;
@property (assign, nonatomic) int reverb;
@property (assign, nonatomic) bool mute;
@property (assign, nonatomic) bool solo;
@end

@class MixerRepresentation;
@protocol MixerRepresentationObserver <NSObject>
- (void)mixer:(MixerRepresentation *)mixer onChannelStatusChange:(MixerChannelRepresentation *)channel;
- (void)mixer:(MixerRepresentation *)mixer onAvailableMidiSourceChange:(NSArray<MidiSourceDescriptionRepresentation *> *)descriptions;
- (void)mixerOnLevelUpdate:(MixerRepresentation *)mixer;
@end

@interface MixerRepresentation : NSObject
@property (readonly, nonatomic) Level level;
- (void)addObserver:(id<MixerRepresentationObserver>)observer;
- (void)removeObserver:(id<MixerRepresentationObserver>)observer;
@end
