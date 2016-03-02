//
//  MidiSourceManagerRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/26/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MidiSourceManagerRepresentation.h"
#import "ObserverList.h"

@interface MidiSourceDescriptionRepresentation ()
- (instancetype)initWithMidiSourceDescription:(MidiSourceDescription *)description;
@end

@implementation MidiSourceDescriptionRepresentation

- (instancetype)initWithMidiSourceDescription:(MidiSourceDescription *)description
{
    self = [super self];
    if (self) {
        _raw = description;
    }
    return self;
}

- (NSString *)name
{
    return [NSString stringWithUTF8String:_raw->name];
}

- (NSString *)filepath
{
    return [NSString stringWithUTF8String:_raw->filepath];
}

- (BOOL)available
{
    return _raw->available;
}

- (int)gain
{
    return _raw->gain;
}

- (int)masterVolume
{
    return _raw->masterVolume;
}

- (MidiSourceDescriptionError)error
{
    return _raw->error;
}

- (BOOL)isEqual:(MidiSourceDescriptionRepresentation *)object
{
    return object->_raw == _raw;
}

@end

@interface MidiSourceManagerRepresentation () {
    MidiSourceManager *_manager;
    ObserverList *_observers;
    NSMutableArray *_descriptions;
    NSMutableArray *_availableDescriptions;
}

- (void)onLoadMidiSourceDescription:(MidiSourceDescription *)description;
- (void)onLoadAvailableMidiSourceDescription:(MidiSourceDescription *)description;
- (void)onUnloadMidiSourceDescription:(MidiSourceDescription *)description;
- (void)onUnloadAvailableMidiSourceDescription:(MidiSourceDescription *)description;
@end

static void onLoadMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
    MidiSourceManagerRepresentation *manager = (__bridge MidiSourceManagerRepresentation *)receiver;
    [manager onLoadMidiSourceDescription:description];
}

static void onLoadAvailableMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
    MidiSourceManagerRepresentation *manager = (__bridge MidiSourceManagerRepresentation *)receiver;
    [manager onLoadAvailableMidiSourceDescription:description];
}

static void onUnloadMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
     MidiSourceManagerRepresentation *manager = (__bridge MidiSourceManagerRepresentation *)receiver;
    [manager onUnloadMidiSourceDescription:description];
}

static void onUnloadAvailableMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
     MidiSourceManagerRepresentation *manager = (__bridge MidiSourceManagerRepresentation *)receiver;
    [manager onUnloadAvailableMidiSourceDescription:description];
}

static MidiSourceManagerObserverCallbacks callbacks = {
    onLoadMidiSourceDescription, onLoadAvailableMidiSourceDescription, onUnloadMidiSourceDescription, onUnloadAvailableMidiSourceDescription
};

@implementation MidiSourceManagerRepresentation

static MidiSourceManagerRepresentation *_sharedInstance = nil;

+ (MidiSourceManagerRepresentation *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[MidiSourceManagerRepresentation alloc] init];
    });
    return _sharedInstance;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _observers = [[ObserverList alloc] init];
        _manager = MidiSourceManagerSharedInstance();
        _descriptions = [NSMutableArray array];
        _availableDescriptions = [NSMutableArray array];
        
        MidiSourceManagerAddObserver(_manager, (__bridge void *)self, &callbacks);
    }
    return self;
}

- (void)addObserver:(id<MidiSourceManagerRepresentationObserver>)observer
{
    [_observers addObserver:observer];
}

- (void)removeObserver:(id<MidiSourceManagerRepresentationObserver>)observer
{
    [_observers removeObserver:observer];
}

- (NSString *)pathForDefaultMidiSource
{
    return [[NSBundle mainBundle] pathForResource:@"GeneralUser GS Live-Audigy v1.44" ofType:@"sf2"];
}

- (void)loadDefaultMidiSourceDescription
{
    [self loadMidiSourceDescriptionFromSoundFont:self.pathForDefaultMidiSource];
}

- (void)loadMidiSourceDescriptionFromSoundFont:(NSString *)filepath
{
    MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(_manager, filepath.UTF8String);
}

- (void)unloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    MidiSourceManagerUnloadMidiSourceDescription(_manager, description.raw);
}

- (void)setGainForDescription:(MidiSourceDescriptionRepresentation *)description gain:(int)gain
{
    MidiSourceManagerSetGainForDescription(_manager, description.raw, gain);
}

- (void)setMasterVolumeForDescription:(MidiSourceDescriptionRepresentation *)description masterVolume:(int)masterVolume
{
    MidiSourceManagerSetMasterVolumeForDescription(_manager, description.raw, masterVolume);
}

- (void)onLoadMidiSourceDescription:(MidiSourceDescription *)description
{
    MidiSourceDescriptionRepresentation *_description = [[MidiSourceDescriptionRepresentation alloc] initWithMidiSourceDescription:description];
    [_descriptions addObject:_description];
    
    [NSThread performBlockOnMainThread:^{
        for (id<MidiSourceManagerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(midiSourceManager:onLoadMidiSourceDescription:)]) {
                [observer midiSourceManager:self onLoadMidiSourceDescription:_description];
            }
        }
    }];
}

- (void)onLoadAvailableMidiSourceDescription:(MidiSourceDescription *)description
{
    MidiSourceDescriptionRepresentation *_description = [[MidiSourceDescriptionRepresentation alloc] initWithMidiSourceDescription:description];
    [_availableDescriptions addObject:_description];
    
    [NSThread performBlockOnMainThread:^{
        for (id<MidiSourceManagerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(midiSourceManager:onLoadAvailableMidiSourceDescription:)]) {
                [observer midiSourceManager:self onLoadAvailableMidiSourceDescription:_description];
            }
        }
    }];
}

- (void)onUnloadMidiSourceDescription:(MidiSourceDescription *)description
{
    MidiSourceDescriptionRepresentation *_description = [[MidiSourceDescriptionRepresentation alloc] initWithMidiSourceDescription:description];
    [_descriptions removeObject:_description];
    
    [NSThread performBlockOnMainThread:^{
        for (id<MidiSourceManagerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(midiSourceManager:onUnloadMidiSourceDescription:)]) {
                [observer midiSourceManager:self onUnloadMidiSourceDescription:_description];
            }
        }
    }];
}

- (void)onUnloadAvailableMidiSourceDescription:(MidiSourceDescription *)description
{
    MidiSourceDescriptionRepresentation *_description = [[MidiSourceDescriptionRepresentation alloc] initWithMidiSourceDescription:description];
    [_availableDescriptions removeObject:_description];
    
    [NSThread performBlockOnMainThread:^{
        for (id<MidiSourceManagerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(midiSourceManager:onUnloadAvailableMidiSourceDescription:)]) {
                [observer midiSourceManager:self onUnloadAvailableMidiSourceDescription:_description];
            }
        }
    }];
}

@end
