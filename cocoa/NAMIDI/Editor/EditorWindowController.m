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

@interface EditorWindowController () <EditorStatusViewControllerDelegate, EditorViewControllerDelegate>
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
    
    self.statusViewControlelr = [[EditorStatusViewController alloc] init];
    _statusViewControlelr.delegate = self;
    _statusViewControlelr.view.translatesAutoresizingMaskIntoConstraints = NO;
    [_tabContainer addSubview:_statusViewControlelr.view];
    [_tabContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[view]|" options:0 metrics:nil views:@{@"view": _statusViewControlelr.view}]];
    [_tabContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[view]|" options:0 metrics:nil views:@{@"view": _statusViewControlelr.view}]];
    
    self.window.contentView.wantsLayer = YES;
    self.window.contentView.layer.masksToBounds = YES;
    
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
    EditorViewController *vc = _controllers[file.identifier];
    vc.view.translatesAutoresizingMaskIntoConstraints = NO;
    [_contentView addSubview:vc.view];
    
    [_contentView removeConstraints:_contentView.constraints];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[view]|" options:0 metrics:nil views:@{@"view": vc.view}]];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[view]|" options:0 metrics:nil views:@{@"view": vc.view}]];
    
    self.currentController = vc;
    
    self.window.titleWithRepresentedFilename = file.filename;
    self.window.representedURL = file.url;
}

- (void)closeFile:(FileRepresentation *)file
{
    // TODO when not save change
    
    self.currentController = nil;
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

#pragma mark EditorStatusViewControllerDelegate

- (void)statusViewController:(EditorStatusViewController *)controller didSelectFile:(FileRepresentation *)file
{
    [self selectFile:file];
}

- (void)statusViewController:(EditorStatusViewController *)controller didPressCloseButten:(FileRepresentation *)file
{
    [self closeFile:file];
}

#pragma mark EditorViewControllerDelegate

- (void)editorViewController:(EditorViewController *)controller didUpdateLine:(NSUInteger)line column:(NSUInteger)column
{
    _statusViewControlelr.positionField.stringValue = [NSString stringWithFormat:@"%lu:%lu", line, column];
}

- (void)editorViewController:(EditorViewController *)controller didPerformCloseAction:(id)sender
{
    [self closeFile:controller.file];
}

#pragma mark Menu Action

- (IBAction)openDocument:(id)sender
{
    [AppController openDocumentForWindow:self.window completion:^(NSURL *url) {
        [self addFileRepresentation:[[FileRepresentation alloc] initWithURL:url]];
    }];
}

- (IBAction)saveDocument:(id)sender
{
    // TODO Track file changes
    
    [_currentController.textView.string writeToURL:_currentController.file.url atomically:YES encoding:NSUTF8StringEncoding error:nil];
}

- (IBAction)saveDocumentAs:(id)sender
{
    [AppController saveDocumentForWindow:self.window completion:^(NSURL *url) {
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
