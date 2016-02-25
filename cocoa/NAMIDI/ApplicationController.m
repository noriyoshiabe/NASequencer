//
//  ApplicationController.m
//  NAMIDI
//
//  Created by abechan on 2/11/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ApplicationController.h"
#import "AboutWindowController.h"
#import "PreferenceWindowController.h"
#import "WelcomeWindowController.h"
#import "GettingStartedWindowController.h"
#import "EditorWindowController.h"
#import "MidiSourceManagerRepresentation.h"

#define kSelectedFileTypeForCreation @"Document.SelectedFileTypeForCreation"

ApplicationController *AppController;

@interface ApplicationController () <NSMenuDelegate, NSOpenSavePanelDelegate>
@property (strong) NSSavePanel *savePanel;
@property (strong) IBOutlet NSView *savePanelAccessoryView;
@property (weak) IBOutlet NSPopUpButton *savePanelFileTypePopupButton;
@property (strong, nonatomic) WelcomeWindowController *welocomeWC;
@property (strong, nonatomic) GettingStartedWindowController *gettingStartedWC;
@property (strong, nonatomic) AboutWindowController *aboutWC;
@property (strong, nonatomic) PreferenceWindowController *preferenceWC;
@property (strong, nonatomic) EditorWindowController *editorWC;
@end


@implementation ApplicationController

- (instancetype)init
{
    self = [super init];
    if (self) {
        AppController = self;
    }
    return self;
}

- (void)awakeFromNib
{
    _savePanelFileTypePopupButton.menu.delegate = self;
}

- (void)initialize
{
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{
                                                              kSelectedFileTypeForCreation: @"nmf",
                                                              }];
    
    NSString *filepath = [[NSBundle mainBundle] pathForResource:@"GeneralUser GS Live-Audigy v1.44" ofType:@"sf2"];
    [[MidiSourceManagerRepresentation sharedInstance] loadMidiSourceDescriptionFromSoundFont:filepath];
}

- (void)showWelcomeWindow
{
    self.welocomeWC = [[WelcomeWindowController alloc] init];
    [_welocomeWC showWindow:self];
}

- (void)closeWelcomeWindow
{
    [_welocomeWC close];
}

- (void)showGettingStartedWindow
{
    self.gettingStartedWC = [[GettingStartedWindowController alloc] init];
    [_gettingStartedWC showWindow:self];
}

- (void)showAboutWindow
{
    if (!_aboutWC) {
        self.aboutWC = [[AboutWindowController alloc] init];
    }
    
    [_aboutWC showWindow:self];
}

- (void)showPreferenceWindow
{
    if (!_preferenceWC) {
        self.preferenceWC = [[PreferenceWindowController alloc] init];
    }
    
    [_preferenceWC showWindow:self];
}

- (void)showEditorWindow
{
    [self showEditorWindowWithFile:nil];
}

- (void)showEditorWindowWithFile:(FileRepresentation *)file
{
    if (!_editorWC) {
        self.editorWC = [[EditorWindowController alloc] init];
    }
    
    [_editorWC showWindow:self];
    
    if (file) {
        [_editorWC addFileRepresentation:file];
    }
}

- (NSArray *)allowedFileTypes
{
    return @[@"nmf", @"abc", @"mml"];
}

- (void)openDocumentWithContentsOfURL:(NSURL *)url
{
    [AppController closeWelcomeWindow];
    
    [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:url display:YES completionHandler:^(NSDocument * _Nullable document, BOOL documentWasAlreadyOpen, NSError * _Nullable error) {
    }];
}

- (void)openDocument
{
    [self openDocumentForWindow:nil completion:^(NSURL *url) {
        [self openDocumentWithContentsOfURL:url];
    }];
}

- (void)openDocumentForWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.allowedFileTypes = self.allowedFileTypes;
    
    if (window) {
        [openPanel beginSheetModalForWindow:window completionHandler:^(NSInteger result) {
            if (NSFileHandlingPanelOKButton == result) {
                completionHandler([openPanel URL]);
            }
        }];
    }
    else {
        if (NSFileHandlingPanelOKButton == [openPanel runModal]) {
            completionHandler([openPanel URL]);
        }
    }
}

- (void)saveDocumentForWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler
{
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    savePanel.allowedFileTypes = self.allowedFileTypes;
    
    if (window) {
        [savePanel beginSheetModalForWindow:window completionHandler:^(NSInteger result) {
            if (NSFileHandlingPanelOKButton == result) {
                completionHandler([savePanel URL]);
            }
        }];
    }
    else {
        if (NSFileHandlingPanelOKButton == [savePanel runModal]) {
            completionHandler([savePanel URL]);
        }
    }
}

- (void)createDocument
{
    [self createDocumentForWindow:nil completion:^(NSURL *url) {
        [self openDocumentWithContentsOfURL:url];
    }];
}

- (void)createDocumentForWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler
{
    self.savePanel = [NSSavePanel savePanel];
    _savePanel.delegate = self;
    _savePanel.prompt = NSLocalizedString(@"Document_Create", @"Create");
    _savePanel.title = NSLocalizedString(@"Document_CreateNewDocument", @"Create New Document");
    _savePanel.nameFieldLabel = NSLocalizedString(@"Document_Filename", @"File Name:");
    
    NSString *ext = [[NSUserDefaults standardUserDefaults] stringForKey:kSelectedFileTypeForCreation];
    [_savePanelFileTypePopupButton selectItemAtIndex: [self.allowedFileTypes indexOfObject:ext]];
    _savePanel.accessoryView = _savePanelAccessoryView;
    _savePanel.nameFieldStringValue = [_savePanel.nameFieldStringValue stringByAppendingPathExtension:ext];
    _savePanel.allowedFileTypes = @[ext];
    
    if (window) {
        [_savePanel beginSheetModalForWindow:window completionHandler:^(NSInteger result) {
            if (NSFileHandlingPanelOKButton == result) {
                // Create empty file
                [[NSData data] writeToURL:[_savePanel URL] atomically:YES];
                
                completionHandler([_savePanel URL]);
                self.savePanel = nil;
            }
        }];
    }
    else {
        // Avoid unwanted moving on NSSavePanel with runModal
        _savePanel.message = @" ";
        _savePanel.styleMask |= NSFullSizeContentViewWindowMask;
        
        if (NSFileHandlingPanelOKButton == [_savePanel runModal]) {
            // Create empty file
            [[NSData data] writeToURL:[_savePanel URL] atomically:YES];
            
            completionHandler([_savePanel URL]);
        }
        
        self.savePanel = nil;
    }
}

#pragma mark NSMenuDelegate

- (void)menuDidClose:(NSMenu *)menu
{
    NSString *ext = menu.highlightedItem.representedObject;
    _savePanel.nameFieldStringValue = [_savePanel.nameFieldStringValue.stringByDeletingPathExtension stringByAppendingPathExtension:ext];
    _savePanel.allowedFileTypes = @[ext];
    [[NSUserDefaults standardUserDefaults] setValue:ext forKey:kSelectedFileTypeForCreation];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

#pragma mark NSOpenSavePanelDelegate

- (nullable NSString *)panel:(id)sender userEnteredFilename:(NSString *)filename confirmed:(BOOL)okFlag
{
    NSString *ext = [[NSUserDefaults standardUserDefaults] stringForKey:kSelectedFileTypeForCreation];
    return [filename.stringByDeletingPathExtension stringByAppendingPathExtension:ext];
}

@end
