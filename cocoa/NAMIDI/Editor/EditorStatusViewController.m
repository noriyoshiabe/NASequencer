//
//  EditorStatusViewController.m
//  NAMIDI
//
//  Created by abechan on 2/15/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EditorStatusViewController.h"
#import "EditorTabItem.h"

@interface EditorStatusViewController () <NSCollectionViewDelegate, NSCollectionViewDataSource, EditorTabItemDelegate> {
    NSIndexPath *draggingPath;
}

@property (weak) IBOutlet NSScrollView *scrollView;
@property (weak) IBOutlet NSCollectionView *collectionView;
@property (weak) IBOutlet NSView *underLine;
@property (weak) FileRepresentation *selectedFile;
@end

@implementation EditorStatusViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _underLine.wantsLayer = YES;
    _underLine.layer.backgroundColor = [NSColor darkGrayColor].CGColor;
    
    _collectionView.delegate = self;
    _collectionView.dataSource = self;
    
    [_collectionView registerForDraggedTypes:@[NSStringPboardType]];
}

- (void)setFiles:(NSMutableArray *)files
{
    _files = files;
    _selectedFile = nil;
}

- (void)selectFile:(FileRepresentation *)file;
{
    _selectedFile = file;
    [_delegate statusViewController:self didSelectFile:file];
    [_collectionView reloadData];
}

#pragma mark NSCollectionViewDataSource

- (NSInteger)collectionView:(NSCollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return _files.count;
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath
{
    EditorTabItem *item = (EditorTabItem *)[collectionView makeItemWithIdentifier:@"EditorTabItem" forIndexPath:indexPath];
    item.delegate = self;
    item.representedObject = _files[indexPath.item];
    item.active = [item.representedObject isEqual:_selectedFile];
    return item;
}

- (NSSet<NSIndexPath *> *)collectionView:(NSCollectionView *)collectionView shouldSelectItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
{
    NSCollectionViewItem *item = [collectionView itemAtIndexPath:indexPaths.anyObject];
    [self selectFile:item.representedObject];
    return nil;
}

- (NSSet<NSIndexPath *> *)collectionView:(NSCollectionView *)collectionView shouldDeselectItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
{
    return nil;
}

#pragma mark NSCollectionViewDelegate

- (id <NSPasteboardWriting>)collectionView:(NSCollectionView *)collectionView pasteboardWriterForItemAtIndexPath:(NSIndexPath *)indexPath {
    NSCollectionViewItem *item = [collectionView itemAtIndex:indexPath.item];
    FileRepresentation *file = item.representedObject;
    return file.url.path;
}

- (void)collectionView:(NSCollectionView *)collectionView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths {
    draggingPath = indexPaths.anyObject;
}

- (void)collectionView:(NSCollectionView *)collectionView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint dragOperation:(NSDragOperation)operation
{
}

- (NSDragOperation)collectionView:(NSCollectionView *)collectionView validateDrop:(id <NSDraggingInfo>)draggingInfo proposedIndexPath:(NSIndexPath **)proposedDropIndexPath dropOperation:(NSCollectionViewDropOperation *)proposedDropOperation {
    
    if (![draggingPath isEqual:*proposedDropIndexPath]) {
        NSInteger fromIndex = draggingPath.item;
        NSInteger toIndex = (*proposedDropIndexPath).item;
        
        if (toIndex < _files.count) {
            FileRepresentation *file = [_files objectAtIndex:fromIndex];
            [_files removeObjectAtIndex:fromIndex];
            [_files insertObject:file atIndex:toIndex];
            [[collectionView animator] moveItemAtIndexPath:draggingPath toIndexPath:*proposedDropIndexPath];
            draggingPath = *proposedDropIndexPath;
        }
    }
    
    return NSDragOperationMove;
}

- (BOOL)collectionView:(NSCollectionView *)collectionView acceptDrop:(id<NSDraggingInfo>)draggingInfo indexPath:(NSIndexPath *)indexPath dropOperation:(NSCollectionViewDropOperation)dropOperation
{
    return YES;
}

#pragma mark EditorTabItemDelegate

- (void)tabItemDidPressCloseButton:(EditorTabItem *)item
{
    [_delegate statusViewController:self didPressCloseButten:item.representedObject];
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
