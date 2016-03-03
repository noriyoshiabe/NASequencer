//
//  MidiSourceManagerRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/26/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MidiSourceManager.h"

@interface MidiSourceDescriptionRepresentation : NSObject
@property (readonly, nonatomic) NSString *name;
@property (readonly, nonatomic) NSString *filepath;
@property (readonly, nonatomic) BOOL available;
@property (readonly, nonatomic) int gain;
@property (readonly, nonatomic) int masterVolume;
@property (readonly, nonatomic) MidiSourceDescriptionError error;
@property (readonly, nonatomic) MidiSourceDescription *raw;
@end

@class MidiSourceManagerRepresentation;
@protocol MidiSourceManagerRepresentationObserver <NSObject>
@optional
- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onLoadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description;
- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onLoadAvailableMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description;
- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onUnloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description;
- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onUnloadAvailableMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description;
- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onReorderMidiSourceDescriptions:(NSArray<MidiSourceDescriptionRepresentation *> *)descriptions availableDescriptions:(NSArray<MidiSourceDescriptionRepresentation *> *)availableDescriptions;
@end

@interface MidiSourceManagerRepresentation : NSObject
@property (readonly, nonatomic) NSArray<MidiSourceDescriptionRepresentation *> *descriptions;
@property (readonly, nonatomic) NSArray<MidiSourceDescriptionRepresentation *> *availableDescriptions;
@property (readonly, nonatomic) NSString *pathForDefaultMidiSource;
+ (MidiSourceManagerRepresentation *)sharedInstance;
- (void)initialize;
- (void)saveMidiSourcePreference;
- (void)addObserver:(id<MidiSourceManagerRepresentationObserver>)observer;
- (void)removeObserver:(id<MidiSourceManagerRepresentationObserver>)observer;
- (void)loadMidiSourceDescriptionFromSoundFont:(NSString *)filepath;
- (void)unloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description;
- (void)setGainForDescription:(MidiSourceDescriptionRepresentation *)description gain:(int)gain;
- (void)setMasterVolumeForDescription:(MidiSourceDescriptionRepresentation *)description masterVolume:(int)masterVolume;
- (void)setReorderdDescriptions:(NSArray *)descriptions;
@end
