//
//  EditorWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EditorWindowController.h"
#import "FileRepresentation.h"

@interface EditorWindowController () <NSCollectionViewDelegate, NSCollectionViewDataSource>
@property (weak) IBOutlet NSScrollView *tabScrollView;
@property (weak) IBOutlet NSCollectionView *tabCollectionView;
@property (weak) IBOutlet NSView *contentView;
@property (strong, nonatomic) NSMutableArray *files;
@end

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

@implementation EditorWindowController

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.files = [NSMutableArray array];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
        [_files addObject:[[FileRepresentation alloc] initWithURL:[NSURL URLWithString:@"file:////Users/abechan/Music/NAMIDI/include/test.nmf"]]];
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
    
    _tabCollectionView.delegate = self;
    _tabCollectionView.dataSource = self;
    [_tabCollectionView reloadData];
    
    [_tabCollectionView selectItemsAtIndexPaths:[NSSet setWithObject:[NSIndexPath indexPathForItem:0 inSection:0]] scrollPosition:NSCollectionViewScrollPositionNone];
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
    [_tabCollectionView deselectItemsAtIndexPaths:_tabCollectionView.selectionIndexPaths];
    [_tabCollectionView selectItemsAtIndexPaths:indexPaths scrollPosition:NSCollectionViewScrollPositionNone];
    return nil;
}

- (NSSet<NSIndexPath *> *)collectionView:(NSCollectionView *)collectionView shouldDeselectItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
{
    return nil;
}

@end
