//
//  Preference.m
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Preference.h"

@interface Preference () {
    NSUserDefaults *_userDefaults;
}

@end

@implementation Preference

static Preference *_sharedInstance = nil;

+ (Preference *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[Preference alloc] init];
    });
    return _sharedInstance;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _userDefaults = [NSUserDefaults standardUserDefaults];
    }
    return self;
}

- (void)initialize
{
    [_userDefaults registerDefaults:self.defaults];
}

- (NSString *)defaultIncludeSearchPath
{
    return [NSApplicationHomeInMusicDirectory() stringByAppendingPathComponent:@"include"];
}

- (NSDictionary *)defaults
{
    return @{
             kShowWelcome: @YES,
             kIncludeSearchPath: self.defaultIncludeSearchPath,
             kSelectedFileTypeForCreation: @"nas",
             kSelectedFileTypeForExport: @"smf",
             kMidiSourceSettings: @[@{kMidiSourceIsDefault: @YES,
                                      kMidiSourceGain: [NSNumber numberWithInt:-100],
                                      kMidiSourceMasterVolume: [NSNumber numberWithInt:0]
                                      }]
             };
}

- (BOOL)showWelcome
{
    return [_userDefaults boolForKey:kShowWelcome];
}

- (NSString *)selectedFileTypeForCreation
{
    return [_userDefaults stringForKey:kSelectedFileTypeForCreation];
}

- (void)setSelectedFileTypeForCreation:(NSString *)selectedFileTypeForCreation
{
    if (selectedFileTypeForCreation) {
        [_userDefaults setValue:selectedFileTypeForCreation forKey:kSelectedFileTypeForCreation];
    }
    else {
        [_userDefaults removeObjectForKey:kSelectedFileTypeForCreation];
    }
    [_userDefaults synchronize];
}

- (NSString *)selectedFileTypeForExport
{
    return [_userDefaults stringForKey:kSelectedFileTypeForExport];
}

- (void)setSelectedFileTypeForExport:(NSString *)selectedFileTypeForExport
{
    if (selectedFileTypeForExport) {
        [_userDefaults setValue:selectedFileTypeForExport forKey:kSelectedFileTypeForExport];
    }
    else {
        [_userDefaults removeObjectForKey:kSelectedFileTypeForExport];
    }
    [_userDefaults synchronize];
}

- (NSString *)externalEditorName
{
    return [_userDefaults stringForKey:kExternaEditorName];
}

- (void)setExternalEditorName:(NSString *)externalEditorName
{
    if (externalEditorName) {
        [_userDefaults setValue:externalEditorName forKey:kExternaEditorName];
    }
    else {
        [_userDefaults removeObjectForKey:kExternaEditorName];
    }
    [_userDefaults synchronize];
}

- (NSString *)includeSearchPath
{
    return [_userDefaults stringForKey:kIncludeSearchPath];
}

- (void)setIncludeSearchPath:(NSString *)includeSearchPath
{
    if (includeSearchPath) {
        [_userDefaults setValue:includeSearchPath forKey:kIncludeSearchPath];
    }
    else {
        [_userDefaults removeObjectForKey:kIncludeSearchPath];
    }
    [_userDefaults synchronize];
}

- (NSData *)includeSearchPathBookmark
{
    return [_userDefaults dataForKey:kIncludeSearchPathBookmark];
}

- (void)setIncludeSearchPathBookmark:(NSData *)includeSearchPathBookmark
{
    if (includeSearchPathBookmark) {
        [_userDefaults setValue:includeSearchPathBookmark forKey:kIncludeSearchPathBookmark];
    }
    else {
        [_userDefaults removeObjectForKey:kIncludeSearchPathBookmark];
    }
    [_userDefaults synchronize];
}

- (NSArray *)midiSourceSettings
{
    return [_userDefaults arrayForKey:kMidiSourceSettings];
}

- (void)setMidiSourceSettings:(NSArray *)midiSourceSettings
{
    if (midiSourceSettings) {
        [_userDefaults setValue:midiSourceSettings forKey:kMidiSourceSettings];
    }
    else {
        [_userDefaults removeObjectForKey:kMidiSourceSettings];
    }
    [_userDefaults synchronize];
}

- (NSString *)lastRootDirectory
{
    return [_userDefaults stringForKey:@"NSNavLastRootDirectory"];
}

- (void)setLastRootDirectory:(NSString *)lastRootDirectory
{
    if (lastRootDirectory) {
        [_userDefaults setValue:lastRootDirectory forKey:@"NSNavLastRootDirectory"];
    }
    else {
        [_userDefaults removeObjectForKey:@"NSNavLastRootDirectory"];
    }
    [_userDefaults synchronize];
}

@end
