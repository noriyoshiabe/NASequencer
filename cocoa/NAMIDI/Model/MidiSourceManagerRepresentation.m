//
//  MidiSourceManagerRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/26/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MidiSourceManagerRepresentation.h"
#import "MidiSourceManager.h"

@interface MidiSourceManagerRepresentation () {
    MidiSourceManager *_manager;
}

@end

@implementation MidiSourceManagerRepresentation

static MidiSourceManagerRepresentation *_sharedInstance = nil;

+ (MidiSourceManagerRepresentation *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[MidiSourceManagerRepresentation alloc] init];
        _sharedInstance->_manager = MidiSourceManagerSharedInstance();
    });
    return _sharedInstance;
}

- (void)loadMidiSourceDescriptionFromSoundFont:(NSString *)filepath
{
    MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(_manager, filepath.UTF8String);
}

@end
