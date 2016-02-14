//
//  EditorWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EditorWindowController.h"
#import "EditorViewController.h"

@interface EditorWindowController () <NSCollectionViewDelegate, NSCollectionViewDataSource>
@property (weak) IBOutlet NSView *tabContainer;
@property (weak) IBOutlet NSScrollView *tabScrollView;
@property (weak) IBOutlet NSCollectionView *tabCollectionView;
@property (weak) IBOutlet NSView *contentView;
@property (strong, nonatomic) NSMutableArray *files;
@property (strong, nonatomic) NSMutableDictionary *controllers;
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

- (void)addFileRepresentation:(FileRepresentation *)file
{
    NSInteger index = [_files indexOfObject:file];
    if (NSNotFound != index) {
        [self selectItemWithIndex:index];
    }
    else {
        [_files addObject:file];
        
        EditorViewController *vc = [[EditorViewController alloc] init];
        vc.file = file;
        [_controllers setObject:vc forKey:file.identifier];
        
        [_tabCollectionView reloadData];
        [self selectItemWithIndex:[_files indexOfObject:file]];
    }
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    _tabContainer.wantsLayer = YES;
    _tabContainer.layer.backgroundColor = [NSColor darkGrayColor].CGColor;
    
    _tabScrollView.hasVerticalScroller = NO;
    _tabCollectionView.wantsLayer = YES;
    _tabCollectionView.layer.backgroundColor = [NSColor lightGrayColor].CGColor;
    
    _tabCollectionView.delegate = self;
    _tabCollectionView.dataSource = self;
}

- (void)selectItemWithIndex:(NSInteger)index
{
    [_tabCollectionView deselectItemsAtIndexPaths:_tabCollectionView.selectionIndexPaths];
    [_tabCollectionView selectItemsAtIndexPaths:[NSSet setWithObject:[NSIndexPath indexPathForItem:index inSection:0]] scrollPosition:NSCollectionViewScrollPositionNone];
    
    FileRepresentation *file = _files[index];
    EditorViewController *vc = _controllers[file.identifier];
    
    [_currentController.view removeFromSuperview];
    vc.view.frame = self.contentView.bounds;
    [self.contentView addSubview:vc.view];
    self.currentController = vc;
}

#pragma mark NSCollectionViewDataSource

- (NSInteger)collectionView:(NSCollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return _files.count;
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath
{
    NSCollectionViewItem *item = [collectionView makeItemWithIdentifier:@"EditorTabItem" forIndexPath:indexPath];
    item.representedObject = _files[indexPath.item];
    return item;
}


- (NSSet<NSIndexPath *> *)collectionView:(NSCollectionView *)collectionView shouldSelectItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
{
    [self selectItemWithIndex:indexPaths.anyObject.item];
    return nil;
}

- (NSSet<NSIndexPath *> *)collectionView:(NSCollectionView *)collectionView shouldDeselectItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
{
    return nil;
}

@end

#pragma mark Prevent scroll bounce

@interface EditorTabScrollView : NSScrollView
@end

@implementation EditorTabScrollView

- (void)scrollWheel:(NSEvent *)theEvent
{
    [super scrollWheel:theEvent];
}

@end

#pragma mark Hide scroll bar

@interface EditorTabScroller : NSScroller
@end

@implementation EditorTabScroller

+ (BOOL)isCompatibleWithOverlayScrollers
{
    return YES;
}

- (void)setHidden:(BOOL)flag
{
    [super setHidden:YES];
}

@end
