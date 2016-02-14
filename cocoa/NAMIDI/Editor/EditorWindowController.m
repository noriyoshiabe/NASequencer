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
@property (strong, nonatomic) NSMutableArray *files;
@end

@implementation EditorWindowController

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.files = [NSMutableArray array];
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
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
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
@end
