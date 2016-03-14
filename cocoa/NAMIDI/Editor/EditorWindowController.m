//
//  EditorWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EditorWindowController.h"
#import "EditorViewController.h"
#import "EditorStatusViewController.h"
#import "ApplicationController.h"

@interface EditorView : NSView
@property (weak, nonatomic) EditorWindowController *controller;
@end

@interface EditorWindowController () <EditorStatusViewControllerDelegate, EditorViewControllerDelegate, NSWindowDelegate>
@property (weak) IBOutlet NSView *tabContainer;
@property (weak) IBOutlet NSView *contentView;
@property (strong, nonatomic) NSMutableArray *files;
@property (strong, nonatomic) NSMutableDictionary *controllers;
@property (strong, nonatomic) EditorStatusViewController *statusViewControlelr;
@property (strong, nonatomic) EditorViewController *currentController;
@end

@implementation EditorWindowController

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.files = [NSMutableArray array];
        self.controllers = [NSMutableDictionary dictionary];
    }
    return self;
}

- (NSString *)windowNibName
{
    return @"EditorWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    self.windowFrameAutosaveName = @"EditorWindowFrame";
    
    self.statusViewControlelr = [[EditorStatusViewController alloc] init];
    _statusViewControlelr.delegate = self;
    [_tabContainer addSubviewWithFitConstraints:_statusViewControlelr.view];
    
    self.window.contentView.wantsLayer = YES;
    self.window.contentView.layer.masksToBounds = YES;
    
    [self.window addTitlebarAccessoryViewController:_statusViewControlelr];
    
    ((EditorView *)self.window.contentView).controller = self;
}

- (void)addFileRepresentation:(FileRepresentation *)file
{
    NSInteger index = [_files indexOfObject:file];
    if (NSNotFound != index) {
        [_statusViewControlelr selectFile:[_files objectAtIndex:index]];
    }
    else {
        [_files addObject:file];
        
        EditorViewController *vc = [[EditorViewController alloc] init];
        vc.delegate = self;
        vc.file = file;
        [_controllers setObject:vc forKey:file.identifier];
        
        _statusViewControlelr.files = _files;
        [_statusViewControlelr selectFile:file];
    }
}

- (void)selectFile:(FileRepresentation *)file
{
    [_currentController.view removeFromSuperview];
    
    EditorViewController *vc = _controllers[file.identifier];
    [_contentView addSubviewWithFitConstraints:vc.view];
    _currentController = vc;
    
    self.window.representedURL = file.url;
    
    [self setWindowTitle];
}

- (void)setWindowTitle
{
    FileRepresentation *file = _currentController.file;
    
    if (_currentController.isDocumentEdited) {
        self.window.documentEdited = YES;
        self.window.title = [NSString stringWithFormat:@"%@ + (%@)", file.filename, file.directory];
    }
    else {
        self.window.documentEdited = NO;
        self.window.title = [NSString stringWithFormat:@"%@ (%@)", file.filename, file.directory];
    }
}

- (void)closeFile:(FileRepresentation *)file
{
    _currentController = nil;
    [_controllers removeObjectForKey:file.identifier];
    [_files removeObject:file];
    
    if (0 == [_files count]) {
        [self close];
    }
    else {
        [self selectFile:_files.firstObject];
        
        _statusViewControlelr.files = _files;
        [_statusViewControlelr selectFile:_files.firstObject];
    }
}

- (void)closeFileWithConfirmation:(FileRepresentation *)file
{
    NSString *message = NSLocalizedString(@"Editor_CloseFileWithConfirmationMessage", @"Do you want to save the changes you made in the document \"%@/%@\" ?");
    NSString *informative = NSLocalizedString(@"Editor_CloseFileWithConfirmationInformative", @"Your changes will be lost if you don't save them.");
    
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = [NSString stringWithFormat:message, file.directory, file.filename];
    alert.informativeText = informative;
    [alert addButtonWithTitle:NSLocalizedString(@"Save", @"Save")];
    [alert addButtonWithTitle:NSLocalizedString(@"Cancel", @"Cancel")];
    [alert addButtonWithTitle:NSLocalizedString(@"DoNotSave", @"Don't Save")];
    [alert beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse returnCode) {
        switch (returnCode) {
            case NSAlertFirstButtonReturn:
                [_currentController saveDocument];
                [self closeFile:file];
                break;
            case NSAlertSecondButtonReturn:
                break;
            case NSAlertThirdButtonReturn:
                [self closeFile:file];
                break;
        }
    }];
}

- (void)revertFileWithConfirmation
{
    FileRepresentation *file = _currentController.file;
    
    NSString *message = NSLocalizedString(@"Editor_RevertFileWithConfirmationMessage", @"Do you want to revert to the most recently saved version of the document \"%@/%@\" ?");
    NSString *informative = NSLocalizedString(@"Editor_RevertFileWithConfirmationInformative", @"Your current changes will be lost.");
    
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = [NSString stringWithFormat:message, file.directory, file.filename];
    alert.informativeText = informative;
    [alert addButtonWithTitle:NSLocalizedString(@"Revert", @"Revert")];
    [alert addButtonWithTitle:NSLocalizedString(@"Cancel", @"Cancel")];
    [alert beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse returnCode) {
        switch (returnCode) {
            case NSAlertFirstButtonReturn:
                [_currentController revertDocument];
                [self setWindowTitle];
                break;
            case NSAlertSecondButtonReturn:
                break;
        }
    }];
}

- (void)reloadFileWithWarning:(EditorViewController *)controller
{
    [controller reloadFile];
    [self setWindowTitle];
    
    FileRepresentation *file = controller.file;
    NSString *informative = NSLocalizedString(@"Editor_ReloadFileWithWarningInformative", @"Reloaded \"%@/%@\" since the file has changed on disk.");
    
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = NSLocalizedString(@"Warning", @"Warning");
    alert.informativeText = [NSString stringWithFormat:informative, file.directory, file.filename];
    [alert addButtonWithTitle:NSLocalizedString(@"Continue", @"Continue")];;
    [alert beginSheetModalForWindow:self.window completionHandler:nil];
}

#pragma mark EditorStatusViewControllerDelegate

- (void)statusViewController:(EditorStatusViewController *)controller didSelectFile:(FileRepresentation *)file
{
    [self selectFile:file];
}

- (void)statusViewController:(EditorStatusViewController *)controller didPressCloseButten:(FileRepresentation *)file
{
    if (_currentController.isDocumentEdited) {
        [self closeFileWithConfirmation:file];
    }
    else {
        [self closeFile:file];
    }
}

#pragma mark EditorViewControllerDelegate

- (void)editorViewController:(EditorViewController *)controller didUpdateLine:(NSUInteger)line column:(NSUInteger)column
{
    _statusViewControlelr.positionField.stringValue = [NSString stringWithFormat:@"%lu:%lu", line, column];
}

- (void)editorViewController:(EditorViewController *)controller didPerformCloseAction:(id)sender
{
    if (_currentController.isDocumentEdited) {
        [self closeFileWithConfirmation:controller.file];
    }
    else {
        [self closeFile:controller.file];
    }
}

- (void)editorViewControllerDidUpdateChangeState:(EditorViewController *)controller
{
    [self setWindowTitle];
}

- (void)editorViewControllerDidPresentedItemDidChange:(EditorViewController *)controller
{
    if (self.window.isKeyWindow) {
        [self reloadFileWithWarning:controller];
        controller.fileChangedOnDisk = NO;
    }
}

#pragma mark Menu Action

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if (@selector(revertDocumentToSaved:) == menuItem.action) {
        return _currentController.isDocumentEdited;
    }
    else if (@selector(selectNextTabViewItem:) == menuItem.action || @selector(selectPreviousTabViewItem:) == menuItem.action) {
        return 1 < _files.count;
    }
    else {
        return YES;
    }
}

- (IBAction)openDocumentInEditor:(id)sender
{
    [AppController openDocumentInEditorWindow:self.window completion:^(NSURL *url) {
        [self addFileRepresentation:[[FileRepresentation alloc] initWithURL:url]];
    }];
}

- (IBAction)saveDocumentAs:(id)sender
{
    [AppController saveDocumentInEditorWindow:self.window filename:_currentController.file.filename completion:^(NSURL *url) {
        if ([_currentController.textView.string writeToURL:url atomically:YES encoding:NSUTF8StringEncoding error:nil]) {
            [self addFileRepresentation:[[FileRepresentation alloc] initWithURL:url]];
        }
    }];
}

- (IBAction)newDocument:(id)sender
{
    [AppController createDocumentForWindow:self.window completion:^(NSURL *url) {
        [self addFileRepresentation:[[FileRepresentation alloc] initWithURL:url]];
    }];
}

- (IBAction)revertDocumentToSaved:(id)sender
{
    [self revertFileWithConfirmation];
}

- (IBAction)selectNextTabViewItem:(id)sender
{
    NSInteger index = [_files indexOfObject:_currentController.file];
    ++index;
    if (_files.count <= index) {
        index = 0;
    }
    [_statusViewControlelr selectFile:[_files objectAtIndex:index]];
}

- (IBAction)selectPreviousTabViewItem:(id)sender
{
    NSInteger index = [_files indexOfObject:_currentController.file];
    --index;
    if (0 > index) {
        index = _files.count - 1;
    }
    [_statusViewControlelr selectFile:[_files objectAtIndex:index]];
}

#pragma NSWindowDelegate

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    for (NSString *key in _controllers) {
        EditorViewController *controller = _controllers[key];
        if (controller.isFileChangedOnDisk) {
            [self reloadFileWithWarning:controller];
            controller.fileChangedOnDisk = NO;
        }
    }
}

@end

#pragma mark Drag and Drop

@implementation EditorView

- (void)awakeFromNib
{
    [self registerForDraggedTypes:@[NSFilenamesPboardType]];
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    NSArray *files = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
    for (NSString *filename in files) {
        if (![AppController.allowedFileTypes containsObject:filename.pathExtension.lowercaseString]) {
            return NSDragOperationNone;
        }
    }
    
    return NSDragOperationCopy;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    NSArray *files = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
    for (NSString *filename in files) {
        NSURL *url = [NSURL fileURLWithPath:filename];
        [_controller addFileRepresentation:[[FileRepresentation alloc] initWithURL:url]];
    }
    return YES;
}

@end
