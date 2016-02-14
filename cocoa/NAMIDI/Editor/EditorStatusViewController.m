//
//  EditorStatusViewController.m
//  NAMIDI
//
//  Created by abechan on 2/15/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EditorStatusViewController.h"

@interface EditorStatusViewController () <NSCollectionViewDelegate, NSCollectionViewDataSource>
@property (weak) IBOutlet NSScrollView *scrollView;
@property (weak) IBOutlet NSCollectionView *collectionView;
@property (weak) IBOutlet NSView *underLine;
@end

@implementation EditorStatusViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _underLine.wantsLayer = YES;
    _underLine.layer.backgroundColor = [NSColor darkGrayColor].CGColor;
    
    _collectionView.delegate = self;
    _collectionView.dataSource = self;
}

- (void)setFiles:(NSArray *)files
{
    _files = files;
    [_collectionView reloadData];
}

- (void)selectFile:(FileRepresentation *)file;
{
    NSUInteger index = [_files indexOfObject:file];
    
    [_collectionView deselectItemsAtIndexPaths:_collectionView.selectionIndexPaths];
    [_collectionView selectItemsAtIndexPaths:[NSSet setWithObject:[NSIndexPath indexPathForItem:index inSection:0]] scrollPosition:NSCollectionViewScrollPositionNone];
    
    [_delegate statusViewController:self didSelectFile:file];
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
    NSCollectionViewItem *item = [collectionView itemAtIndex:indexPaths.anyObject.item];
    [self selectFile:item.representedObject];
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
