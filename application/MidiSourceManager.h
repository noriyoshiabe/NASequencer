#pragma once

#import <Foundation/Foundation.h>

#import "MidiSource.h"

typedef NS_ENUM (NSUInteger, MidiSourceDescriptionError) {
    MidiSourceDescriptionErrorNoError,
    MidiSourceDescriptionErrorFileNotFound,
    MidiSourceDescriptionErrorUnsupportedVersion,
    MidiSourceDescriptionErrorInvalidFileFormat,
};

@interface MidiSourceDescription : NSObject

@property (nonatomic, readonly) NSString *name;
@property (nonatomic, readonly) NSString *filepath;
@property (nonatomic, readonly) bool available;
@property (nonatomic, readonly) MidiSourceDescriptionError error;

@end

@protocol MidiSourceManagerObserver;

@interface MidiSourceManager : NSObject

@property (nonatomic, readonly) NSArray *descriptions;

+ (MidiSourceManager *)sharedInstance;

- (void)loadSoundFont:(NSString *)filepath;
- (void)unloadMidiSource:(MidiSourceDescription *)description;

- (MidiSource *)createMidiSource:(MidiSourceDescription *)description;

- (void)addObserver:(id<MidiSourceManagerObserver>)observer;
- (void)removeObserver:(id<MidiSourceManagerObserver>)observer;

@end

@protocol MidiSourceManagerObserver <NSObject>
@optional
- (void)manager:(MidiSourceManager *)manager onLoadMidiSource:(MidiSourceDescription *)description;
- (void)manager:(MidiSourceManager *)manager onUnloadMidiSource:(MidiSourceDescription *)description;
@end
