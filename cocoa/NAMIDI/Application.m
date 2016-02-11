//
//  Application.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Application.h"
#import "ApplicationController.h"
#import "Welcome.h"

#define kSelectedFileTypeForCreation @"Document.SelectedFileTypeForCreation"

@interface Application () <NSMenuDelegate, NSOpenSavePanelDelegate>
@property (strong) NSSavePanel *savePanel;
@property (strong) IBOutlet NSView *savePanelAccessoryView;
@property (weak) IBOutlet NSPopUpButton *savePanelFileTypePopupButton;
@end

@implementation Application

- (void)finishLaunching
{
    [super finishLaunching];
    _savePanelFileTypePopupButton.menu.delegate = self;
    
    if (![[NSUserDefaults standardUserDefaults] stringForKey:kSelectedFileTypeForCreation]) {
        [[NSUserDefaults standardUserDefaults] setValue:@"nmf" forKey:kSelectedFileTypeForCreation];
        [[NSUserDefaults standardUserDefaults] synchronize];
    }
}

- (void)orderFrontStandardAboutPanel:(id)sender
{
    [[ApplicationController sharedInstance] showAboutWindow];
}

- (void)openDocumentWithContentsOfURL:(NSURL *)url
{
    [[Welcome sharedInstance] closeWindow];
    
    [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:url display:YES completionHandler:^(NSDocument * _Nullable document, BOOL documentWasAlreadyOpen, NSError * _Nullable error) {
    }];
}

- (NSArray *)allowedFileTypes
{
    return @[@"nmf", @"abc", @"mml"];
}

- (void)openDocument
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.allowedFileTypes = self.allowedFileTypes;
    
    if (NSFileHandlingPanelOKButton == [openPanel runModal]) {
        [self openDocumentWithContentsOfURL:[openPanel URL]];
    }
}

- (void)createDocument
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
    
    // Avoid unwanted moving on NSSavePanel with runModal
    _savePanel.message = @" ";
    _savePanel.styleMask |= NSFullSizeContentViewWindowMask;
    
    if (NSFileHandlingPanelOKButton == [_savePanel runModal]) {
        // Create empty file
        [[NSData data] writeToURL:[_savePanel URL] atomically:YES];
        
        [self openDocumentWithContentsOfURL:[_savePanel URL]];
    }
    
    _savePanel.accessoryView = nil;
    _savePanel.delegate = nil;
    self.savePanel = nil;
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
