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
    
    self.statusViewControlelr = [[EditorStatusViewController alloc] init];
    _statusViewControlelr.layoutAttribute = NSLayoutAttributeBottom;
    _statusViewControlelr.delegate = self;
    [self.window addTitlebarAccessoryViewController:_statusViewControlelr];
    
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
    self.window.contentViewController = _controllers[file.identifier];
}

#pragma mark EditorViewControllerDelegate

- (void)editorViewController:(EditorViewController *)controller didUpdateLine:(NSUInteger)line column:(NSUInteger)column
{
    _statusViewControlelr.potitionField.stringValue = [NSString stringWithFormat:@"%lu:%lu", line, column];
}

@end
