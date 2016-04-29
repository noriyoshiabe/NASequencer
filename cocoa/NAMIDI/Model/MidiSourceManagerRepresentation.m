//
//  MidiSourceManagerRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/26/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MidiSourceManagerRepresentation.h"
#import "ObserverList.h"
#import "Preference.h"

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
- (void)onReorderMidiSourceDescriptions:(NAArray *)descriptions availableDescriptions:(NAArray *)availableDescriptions;
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

static void onReorderMidiSourceDescriptions(void *receiver, NAArray *descriptions, NAArray *availableDescriptions)
{
    MidiSourceManagerRepresentation *manager = (__bridge MidiSourceManagerRepresentation *)receiver;
    [manager onReorderMidiSourceDescriptions:descriptions availableDescriptions:availableDescriptions];
}

static MidiSourceManagerObserverCallbacks callbacks = {
    onLoadMidiSourceDescription, onLoadAvailableMidiSourceDescription, onUnloadMidiSourceDescription, onUnloadAvailableMidiSourceDescription, onReorderMidiSourceDescriptions
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

- (void)initialize
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        for (NSDictionary *settings in [Preference sharedInstance].midiSourceSettings) {
            NSNumber *isDefault = settings[kMidiSourceIsDefault];
            
            if (isDefault.boolValue) {
                [self loadMidiSourceDescriptionFromSoundFont:self.pathForDefaultMidiSource];
            }
            else {
                NSData *bookmark = settings[kMidiSourceBookmark];
                if (bookmark) {
                    NSError *error = nil;
                    NSURL *bookmarkedURL = [NSURL URLByResolvingBookmarkData:bookmark options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:nil error:&error];
                    [bookmarkedURL startAccessingSecurityScopedResource];
                }
                
                NSString *filepath = settings[kMidiSourceFilePath];
                [self loadMidiSourceDescriptionFromSoundFont:filepath];
            }
            
            MidiSourceDescriptionRepresentation *description = _descriptions.lastObject;
            description.settings = settings;
            
            NSNumber *gain = settings[kMidiSourceGain];
            [self setGainForDescription:description gain:gain.intValue];
            NSNumber *masterVolume = settings[kMidiSourceMasterVolume];
            [self setMasterVolumeForDescription:description masterVolume:masterVolume.intValue];
        }
    });
}

- (NSString *)pathForDefaultMidiSource
{
    NSString *path = [[NSBundle mainBundle] pathForResource:@"GeneralUser GS Live-Audigy v1.44" ofType:@"sf2"];
    if (!path) {
        // Care for that default soundfont deleted by fuckin' user
        path = [[NSBundle mainBundle].resourcePath stringByAppendingString:@"/GeneralUser GS Live-Audigy v1.44.sf2"];
    }
    return path;
}

- (void)saveMidiSourcePreference
{
    NSMutableArray *settings = [NSMutableArray array];
    for (MidiSourceDescriptionRepresentation *description in _descriptions) {
        if (description.available) {
            BOOL isDefault = [description.filepath isEqualToString:self.pathForDefaultMidiSource];
            
            NSError *error = nil;
            NSData *bookmark = [[NSURL fileURLWithPath:description.filepath] bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:&error];
            
            [settings addObject:@{
                                  kMidiSourceFilePath: description.filepath,
                                  kMidiSourceName: description.name,
                                  kMidiSourceGain: [NSNumber numberWithInt:description.gain],
                                  kMidiSourceMasterVolume: [NSNumber numberWithInt:description.masterVolume],
                                  kMidiSourceBookmark: bookmark ? bookmark : [NSData data],
                                  kMidiSourceIsDefault: [NSNumber numberWithBool:isDefault]
                                  }];
        }
        else if (description.settings) {
            [settings addObject:description.settings];
        }
    }
    
    [Preference sharedInstance].midiSourceSettings = settings;
}

- (bool)loadMidiSourceDescriptionFromSoundFont:(NSString *)filepath
{
    return MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(_manager, filepath.UTF8String);
}

- (void)unloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    MidiSourceManagerUnloadMidiSourceDescription(_manager, description.raw);
}

- (bool)reloadMidiSourceDescription:(MidiSourceDescriptionRepresentation *)description
{
    NSData *bookmark = description.settings[kMidiSourceBookmark];
    if (bookmark) {
        NSError *error = nil;
        NSURL *bookmarkedURL = [NSURL URLByResolvingBookmarkData:bookmark options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:nil error:&error];
        [bookmarkedURL startAccessingSecurityScopedResource];
    }
    
    return MidiSourceManagerReloadMidiSourceDescription(_manager, description.raw);
}

- (void)setGainForDescription:(MidiSourceDescriptionRepresentation *)description gain:(int)gain
{
    MidiSourceManagerSetGainForDescription(_manager, description.raw, gain);
}

- (void)setMasterVolumeForDescription:(MidiSourceDescriptionRepresentation *)description masterVolume:(int)masterVolume
{
    MidiSourceManagerSetMasterVolumeForDescription(_manager, description.raw, masterVolume);
}

- (void)setReorderdDescriptions:(NSArray *)descriptions
{
    NAArray *__descriptions = NAArrayCreate(4, NADescriptionAddress);
    for (MidiSourceDescriptionRepresentation *description in descriptions) {
        NAArrayAppend(__descriptions, description.raw);
    }
    MidiSourceManagerSetReorderedDescriptions(_manager, __descriptions);
    NAArrayDestroy(__descriptions);
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

- (void)onReorderMidiSourceDescriptions:(NAArray *)descriptions availableDescriptions:(NAArray *)availableDescriptions
{
    [_descriptions sortUsingComparator:^NSComparisonResult(MidiSourceDescriptionRepresentation *obj1, MidiSourceDescriptionRepresentation *obj2) {
        return NAArrayFindFirstIndex(descriptions, obj1.raw, NAArrayAddressComparator) - NAArrayFindFirstIndex(descriptions, obj2.raw, NAArrayAddressComparator);
    }];
    
    [_availableDescriptions sortUsingComparator:^NSComparisonResult(MidiSourceDescriptionRepresentation *obj1, MidiSourceDescriptionRepresentation *obj2) {
        return NAArrayFindFirstIndex(availableDescriptions, obj1.raw, NAArrayAddressComparator) - NAArrayFindFirstIndex(availableDescriptions, obj2.raw, NAArrayAddressComparator);
    }];
    
    [NSThread performBlockOnMainThread:^{
        for (id<MidiSourceManagerRepresentationObserver> observer in _observers) {
            if ([observer respondsToSelector:@selector(midiSourceManager:onReorderMidiSourceDescriptions:availableDescriptions:)]) {
                [observer midiSourceManager:self onReorderMidiSourceDescriptions:_descriptions availableDescriptions:_availableDescriptions];
            }
        }
    }];
}

@end
