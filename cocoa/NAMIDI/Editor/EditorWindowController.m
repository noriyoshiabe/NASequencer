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

@interface EditorWindowController () <EditorStatusViewControllerDelegate, EditorViewControllerDelegate>
@property (weak) IBOutlet NSView *tabContainer;
@property (weak) IBOutlet NSView *contentView;
@property (strong, nonatomic) NSMutableArray *files;
@property (strong, nonatomic) NSMutableDictionary *controllers;
@property (strong, nonatomic) EditorStatusViewController *statusViewControlelr;
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
    
    //self.window.contentViewController.view = self.contentView;
    
    self.statusViewControlelr = [[EditorStatusViewController alloc] init];
    _statusViewControlelr.delegate = self;
    _statusViewControlelr.view.translatesAutoresizingMaskIntoConstraints = NO;
    [_tabContainer addSubview:_statusViewControlelr.view];
    [_tabContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[view]|" options:0 metrics:nil views:@{@"view": _statusViewControlelr.view}]];
    [_tabContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[view]|" options:0 metrics:nil views:@{@"view": _statusViewControlelr.view}]];
    
    self.window.contentView.wantsLayer = YES;
    self.window.contentView.layer.cornerRadius = 4.0;
    self.window.contentView.layer.masksToBounds = YES;
}

- (void)addFileRepresentation:(FileRepresentation *)file
{
    NSInteger index = [_files indexOfObject:file];
    if (NSNotFound != index) {
        [_statusViewControlelr selectFile:file];
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

#pragma mark EditorStatusViewControllerDelegate

- (void)statusViewController:(EditorStatusViewController *)controller didSelectFile:(FileRepresentation *)file
{
    EditorViewController *vc = _controllers[file.identifier];
    vc.view.translatesAutoresizingMaskIntoConstraints = NO;
    [_contentView addSubview:vc.view];
    
    [_contentView removeConstraints:_contentView.constraints];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[view]|" options:0 metrics:nil views:@{@"view": vc.view}]];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[view]|" options:0 metrics:nil views:@{@"view": vc.view}]];
}

#pragma mark EditorViewControllerDelegate

- (void)editorViewController:(EditorViewController *)controller didUpdateLine:(NSUInteger)line column:(NSUInteger)column
{
    _statusViewControlelr.potitionField.stringValue = [NSString stringWithFormat:@"%lu:%lu", line, column];
}

@end
