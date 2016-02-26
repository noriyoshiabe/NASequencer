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
@property (readonly, nonatomic) MidiSourceDescriptionError error;
@end

@class MidiSourceManagerRepresentation;
@protocol MidiSourceManagerRepresentationObserver <NSObject>
@optional
- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onLoadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description;
- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onLoadAvailableMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description;
- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onUnloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description;
- (void)midiSourceManager:(MidiSourceManagerRepresentation *)manager onUnloadAvailableMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description;
@end

@interface MidiSourceManagerRepresentation : NSObject
@property (readonly, nonatomic) NSArray *descriptions;
@property (readonly, nonatomic) NSArray *availableDescriptions;
+ (MidiSourceManagerRepresentation *)sharedInstance;
- (void)addObserver:(id<MidiSourceManagerRepresentationObserver>)observer;
- (void)removeObserver:(id<MidiSourceManagerRepresentationObserver>)observer;
- (void)loadMidiSourceDescriptionFromSoundFont:(NSString *)filepath;
@end
