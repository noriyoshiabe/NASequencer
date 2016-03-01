//
//  Preference.m
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Preference.h"

#define kShowWelcome @"ShowWelcome"
#define kExternaEditorName @"ExternalEditorName"
#define kIncludeSearchPath @"IncludeSearchPath"

#define kSelectedFileTypeForCreation @"SelectedFileTypeForCreation"
#define kSelectedFileTypeForExport @"SelectedFileTypeForExport"

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

- (NSDictionary *)defaults
{
    return @{
             kShowWelcome: @YES,
             kIncludeSearchPath: [NSUserMusicDirectory() stringByAppendingPathComponent:@"NAMIDI/include"],
             kSelectedFileTypeForCreation: @"nmf",
             kSelectedFileTypeForExport: @"smf",
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

@end
