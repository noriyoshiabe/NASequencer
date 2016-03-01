//
//  GeneralViewController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "GeneralViewController.h"
#import "Preference.h"

@interface GeneralViewController () <NSOpenSavePanelDelegate> {
    NSString *_externalEditorName;
}
@property (assign, nonatomic) BOOL useExternalEditor;
@property (readonly, nonatomic) NSImage *editorIconImage;
@property (readonly, nonatomic) NSString *editorName;
@property (readonly, nonatomic) NSString *editorSelectButtonLabel;
@end

@implementation GeneralViewController

- (NSString *)identifier
{
    return @"General";
}

- (NSImage *)toolbarItemImage
{
    return [NSImage imageNamed:NSImageNamePreferencesGeneral];
}

- (NSString *)toolbarItemLabel
{
    return NSLocalizedString(@"Preference_General", @"General");
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _externalEditorName = [Preference sharedInstance].externalEditorName;
    if (_externalEditorName) {
        self.useExternalEditor = YES;
    }
}

- (void)setUseExternalEditor:(BOOL)useExternalEditor
{
    _useExternalEditor = useExternalEditor;
    if (!_useExternalEditor) {
        [Preference sharedInstance].externalEditorName = nil;
        _externalEditorName = nil;
    }
    
    [self refreshEditorSettings];
}

- (void)refreshEditorSettings
{
    [self willChangeValueForKey:@"editorName"];
    [self didChangeValueForKey:@"editorName"];
    [self willChangeValueForKey:@"editorIconImage"];
    [self didChangeValueForKey:@"editorIconImage"];
    [self willChangeValueForKey:@"editorSelectButtonLabel"];
    [self didChangeValueForKey:@"editorSelectButtonLabel"];
}

- (NSImage *)editorIconImage
{
    if (_useExternalEditor && _externalEditorName) {
        NSString *appPath = [[NSWorkspace sharedWorkspace] fullPathForApplication:_externalEditorName];
        return [[NSWorkspace sharedWorkspace] iconForFile:appPath];
    }
    else {
        return [NSApp applicationIconImage];
    }
}

- (NSString *)editorName
{
    return _useExternalEditor && _externalEditorName ? _externalEditorName : @"Internal Editor";
}

- (NSString *)editorSelectButtonLabel
{
    return _useExternalEditor && _externalEditorName ? @"Change" : @"Select";
}

- (IBAction)editorSelectButtonPressed:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.directoryURL = [NSURL fileURLWithPath:@"/Applications" isDirectory:YES];
    openPanel.delegate = self;
    
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        if (NSFileHandlingPanelOKButton == result) {
            NSBundle *appBundle = [NSBundle bundleWithURL:openPanel.URL];
            _externalEditorName = [appBundle objectForInfoDictionaryKey:@"CFBundleName"];
            [Preference sharedInstance].externalEditorName = _externalEditorName;
            [self refreshEditorSettings];
        }
    }];
}

#pragma mark NSOpenSavePanelDelegate

- (BOOL)panel:(id)sender shouldEnableURL:(NSURL *)url
{
    if ([url.pathExtension isEqualToString:@"app"]) {
        return YES;
    }
    
    BOOL ret;
    [[NSFileManager defaultManager] fileExistsAtPath:url.path isDirectory:&ret];
    return ret;
}

@end
