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
#import "MidiSourceManagerRepresentation.h"
#import "ExportWindowController.h"
#import "MainWindowController.h"
#import "Preference.h"
#import "Document.h"

ApplicationController *AppController;

@interface ApplicationController () <NSMenuDelegate, NSOpenSavePanelDelegate, ExportWindowControllerDelegate> {
    NSMutableArray *_exportControllers;
    NSMutableArray *_exampleDocuments;
}

@property (strong) NSSavePanel *savePanel;
@property (strong) IBOutlet NSView *savePanelAccessoryView;
@property (weak) IBOutlet NSPopUpButton *savePanelFileTypePopupButton;
@property (strong) NSSavePanel *exportPanel;
@property (strong) IBOutlet NSView *exportPanelAccessoryView;
@property (weak) IBOutlet NSPopUpButton *exportPanelFileTypePopoupButton;
@property (strong, nonatomic) WelcomeWindowController *welocomeWC;
@property (strong, nonatomic) GettingStartedWindowController *gettingStartedWC;
@property (strong, nonatomic) AboutWindowController *aboutWC;
@property (strong, nonatomic) PreferenceWindowController *preferenceWC;
@end


@implementation ApplicationController

- (instancetype)init
{
    self = [super init];
    if (self) {
        AppController = self;
        _exportControllers = [NSMutableArray array];
        _exampleDocuments = [NSMutableArray array];
    }
    return self;
}

- (void)awakeFromNib
{
    _savePanelFileTypePopupButton.menu.delegate = self;
    _exportPanelFileTypePopoupButton.menu.delegate = self;
}

- (void)initialize
{
    [[Preference sharedInstance] initialize];
    
    [self createDefaultIncludeDirectory];
    
    NSData *bookmark = [Preference sharedInstance].includeSearchPathBookmark;
    if (bookmark) {
        NSError *error = nil;
        NSURL *bookmarkedURL = [NSURL URLByResolvingBookmarkData:bookmark options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:nil error:&error];
        [bookmarkedURL startAccessingSecurityScopedResource];
    }
    
    [[MidiSourceManagerRepresentation sharedInstance] initialize];
}

- (void)createDefaultIncludeDirectory
{
    NSString *includeSearchPath = [Preference sharedInstance].includeSearchPath;
    if ([includeSearchPath isEqualToString:[Preference sharedInstance].defaultIncludeSearchPath]) {
        if (![[NSFileManager defaultManager] fileExistsAtPath:includeSearchPath isDirectory:NULL]) {
            [[NSFileManager defaultManager] createDirectoryAtPath:includeSearchPath withIntermediateDirectories:YES attributes:nil error:nil];
        }
    }
}

- (NSString *)exampleDirectoryPath
{
    return [NSApplicationHomeInMusicDirectory() stringByAppendingPathComponent:@"example"];
}

- (void)createExampleDirectory
{
    if (![[NSFileManager defaultManager] fileExistsAtPath:self.exampleDirectoryPath isDirectory:NULL]) {
        [[NSFileManager defaultManager] createDirectoryAtPath:self.exampleDirectoryPath withIntermediateDirectories:YES attributes:nil error:nil];
    }
}

- (BOOL)needShowWelcome
{
    return ![NSApplication sharedApplication].keyWindow && ![NSDocumentController sharedDocumentController].currentDocument &&  [Preference sharedInstance].showWelcome;
}

- (void)showWelcomeWindow
{
    _welocomeWC = [[WelcomeWindowController alloc] init];
    [_welocomeWC showWindow:self];
    [_welocomeWC.window center];
}

- (void)closeWelcomeWindow
{
    [_welocomeWC close];
}

- (void)showGettingStartedWindow
{
    _gettingStartedWC = [[GettingStartedWindowController alloc] init];
    [_gettingStartedWC showWindow:self];
    [_gettingStartedWC.window center];
}

- (void)showAboutWindow
{
    if (!_aboutWC) {
        _aboutWC = [[AboutWindowController alloc] init];
    }
    
    [_aboutWC showWindow:self];
    [_aboutWC.window setFlippedFrameTopLeftPoint:CGPointMake(100, 100)];
}

- (void)showPreferenceWindow
{
    if (!_preferenceWC) {
        _preferenceWC = [[PreferenceWindowController alloc] init];
    }
    
    [_preferenceWC showWindow:self];
    [_preferenceWC.window setFlippedFrameTopLeftPoint:CGPointMake(200, 200)];
}

- (NSArray *)allowedFileTypes
{
    return @[@"nas", @"abc", @"mml"];
}

- (NSArray *)allowedFileTypesInEditor
{
    return @[@"nas", @"abc", @"abh", @"mml"];
}

- (void)openDocumentWithContentsOfURL:(NSURL *)url
{
    [AppController closeWelcomeWindow];
    
    [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:url display:YES completionHandler:^(NSDocument * _Nullable document, BOOL documentWasAlreadyOpen, NSError * _Nullable error) {
        
        // Avoid editor window hidden
        MainWindowController *mainVC = document.windowControllers[0];
        [mainVC showEditorWindow];
    }];
}

- (void)openDocument
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.allowedFileTypes = self.allowedFileTypes;
    
    if (NSFileHandlingPanelOKButton == [openPanel runModal]) {
        [self openDocumentWithContentsOfURL:openPanel.URL];
    }
}

- (void)openDocumentInEditorWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.allowedFileTypes = self.allowedFileTypesInEditor;
    
    [openPanel beginSheetModalForWindow:window completionHandler:^(NSInteger result) {
        if (NSFileHandlingPanelOKButton == result) {
            completionHandler(openPanel.URL);
        }
    }];
}

- (void)saveDocumentInEditorWindow:(NSWindow *)window filename:(NSString *)filename completion:(void (^)(NSURL *url))completionHandler
{
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    savePanel.nameFieldStringValue = filename;
    savePanel.allowedFileTypes = @[filename.pathExtension];
    
    [savePanel beginSheetModalForWindow:window completionHandler:^(NSInteger result) {
        if (NSFileHandlingPanelOKButton == result) {
            completionHandler(savePanel.URL);
        }
    }];
}

- (void)createDocument
{
    [self createDocumentForWindow:nil completion:^(NSURL *url) {
        [self openDocumentWithContentsOfURL:url];
    }];
}

- (void)createDocumentForWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler
{
    _savePanel = [NSSavePanel savePanel];
    _savePanel.delegate = self;
    _savePanel.prompt = NSLocalizedString(@"Document_Create", @"Create");
    _savePanel.title = NSLocalizedString(@"Document_CreateNewDocument", @"Create New Document");
    _savePanel.nameFieldLabel = NSLocalizedString(@"Document_Filename", @"File Name:");
    
    NSString *ext = [Preference sharedInstance].selectedFileTypeForCreation;
    [_savePanelFileTypePopupButton selectItemAtIndex: [self.allowedFileTypes indexOfObject:ext]];
    _savePanel.accessoryView = _savePanelAccessoryView;
    _savePanel.nameFieldStringValue = [_savePanel.nameFieldStringValue stringByAppendingPathExtension:ext];
    _savePanel.allowedFileTypes = @[ext];
    
    if (window) {
        [_savePanel beginSheetModalForWindow:window completionHandler:^(NSInteger result) {
            if (NSFileHandlingPanelOKButton == result) {
                // Create empty file
                [[NSData data] writeToURL:_savePanel.URL atomically:YES];
                
                completionHandler(_savePanel.URL);
                _savePanel = nil;
            }
        }];
    }
    else {
        // Avoid unwanted moving on NSSavePanel with runModal
        _savePanel.message = @" ";
        _savePanel.styleMask |= NSFullSizeContentViewWindowMask;
        
        if (NSFileHandlingPanelOKButton == [_savePanel runModal]) {
            // Create empty file
            [[NSData data] writeToURL:_savePanel.URL atomically:YES];
            
            completionHandler(_savePanel.URL);
        }
        
        _savePanel = nil;
    }
}

- (void)openExampleDocument:(NSString *)fileType
{
    [self createExampleDirectory];
    
    NSString *filename = [NSString stringWithFormat:@"example.%@", fileType];
    NSURL *srcUrl = [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"example" ofType:fileType]];
    NSURL *dstUrl = [NSURL fileURLWithPath:[self.exampleDirectoryPath stringByAppendingPathComponent:filename]];
    
    [[NSString stringWithContentsOfURL:srcUrl encoding:NSUTF8StringEncoding error:nil] writeToURL:dstUrl atomically:YES encoding:NSUTF8StringEncoding error:nil];
    
    [self openDocumentWithContentsOfURL:dstUrl];
}

- (NSArray *)allowedExportFileTypes
{
    return @[@"smf", @"wav", @"aac"];
}

- (void)exportDocumentForWindow:(NSWindow *)window file:(FileRepresentation *)file
{
    _exportPanel = [NSSavePanel savePanel];
    _exportPanel.delegate = self;
    _exportPanel.prompt = NSLocalizedString(@"Document_Export", @"Export");
    _exportPanel.title = NSLocalizedString(@"Document_ExportDocument", @"Export Document");
    _exportPanel.nameFieldLabel = NSLocalizedString(@"Document_Filename", @"File Name:");
    
    NSString *ext = [Preference sharedInstance].selectedFileTypeForExport;
    [_exportPanelFileTypePopoupButton selectItemAtIndex: [self.allowedExportFileTypes indexOfObject:ext]];
    _exportPanel.accessoryView = _exportPanelAccessoryView;
    _exportPanel.nameFieldStringValue = [_exportPanel.nameFieldStringValue stringByAppendingPathExtension:ext];
    _exportPanel.allowedFileTypes = @[ext];
    
    [_exportPanel beginSheetModalForWindow:window completionHandler:^(NSInteger result) {
        if (NSFileHandlingPanelOKButton == result) {
            ExportWindowController *exportWC = [[ExportWindowController alloc] init];
            exportWC.file = file;
            exportWC.outputUrl = _exportPanel.URL;
            exportWC.delegate = self;
            
            CGRect frame = window.frame;
            frame.origin.x = frame.origin.x + frame.size.width - CGRectGetWidth(exportWC.window.frame) - 10;
            frame.origin.y = frame.origin.y + frame.size.height - CGRectGetHeight(exportWC.window.frame) - 65;
            frame.size = exportWC.window.frame.size;
            [exportWC.window setFrame:frame display:YES];
            [exportWC showWindow:self];
            
            [_exportControllers addObject:exportWC];
            
            _exportPanel = nil;
        }
    }];
}

#pragma mark ExportWindowControllerDelegate

-(void)exportControllerDidClose:(ExportWindowController *)controller
{
    [_exportControllers removeObject:controller];
}

#pragma mark NSMenuDelegate

- (void)menuDidClose:(NSMenu *)menu
{
    NSString *ext = menu.highlightedItem.representedObject;
    
    if (_savePanel) {
        _savePanel.nameFieldStringValue = [_savePanel.nameFieldStringValue.stringByDeletingPathExtension stringByAppendingPathExtension:ext];
        _savePanel.allowedFileTypes = @[ext];
        [Preference sharedInstance].selectedFileTypeForCreation = ext;
    }
    else if (_exportPanel) {
        _exportPanel.nameFieldStringValue = [_exportPanel.nameFieldStringValue.stringByDeletingPathExtension stringByAppendingPathExtension:ext];
        _exportPanel.allowedFileTypes = @[ext];
        [Preference sharedInstance].selectedFileTypeForExport = ext;
    }
}

#pragma mark NSOpenSavePanelDelegate

- (nullable NSString *)panel:(id)sender userEnteredFilename:(NSString *)filename confirmed:(BOOL)okFlag
{
    if (_savePanel) {
        return [filename.stringByDeletingPathExtension stringByAppendingPathExtension:[Preference sharedInstance].selectedFileTypeForCreation];
    }
    else if (_exportPanel) {
        return [filename.stringByDeletingPathExtension stringByAppendingPathExtension:[Preference sharedInstance].selectedFileTypeForExport];
    }
    else {
        return nil;
    }
}

@end
