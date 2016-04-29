//
//  Preference.h
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

#define kShowWelcome @"ShowWelcome"
#define kExternaEditorName @"ExternalEditorName"
#define kIncludeSearchPath @"IncludeSearchPath"
#define kIncludeSearchPathBookmark @"IncludeSearchPathBookmark"
#define kMidiSourceSettings @"MidiSourceSettings"
#define kMidiSourceFilePath @"MidiSourceFilePath"
#define kMidiSourceName @"MidiSourceName"
#define kMidiSourceGain @"MidiSourceGain"
#define kMidiSourceMasterVolume @"MidiSourceMasterVolume"
#define kMidiSourceBookmark @"MidiSourceBookmark"
#define kMidiSourceIsDefault @"MidiSourceIsDefault"

#define kSelectedFileTypeForCreation @"SelectedFileTypeForCreation"
#define kSelectedFileTypeForExport @"SelectedFileTypeForExport"

#define kSuppressedNewVersion @"SuppressedNewVersion"

@interface Preference : NSObject
@property (readonly, nonatomic) BOOL showWelcome;
@property (readwrite, nonatomic) NSString *selectedFileTypeForCreation;
@property (readwrite, nonatomic) NSString *selectedFileTypeForExport;
@property (readwrite, nonatomic) NSString *externalEditorName;
@property (readwrite, nonatomic) NSString *includeSearchPath;
@property (readwrite, nonatomic) NSData *includeSearchPathBookmark;
@property (readwrite, nonatomic) NSArray *midiSourceSettings;
@property (readwrite, nonatomic) NSString *lastRootDirectory;
@property (readwrite, nonatomic) NSString *suppressedNewVersion;
+ (Preference *)sharedInstance;
- (void)initialize;
- (NSString *)defaultIncludeSearchPath;
@end
