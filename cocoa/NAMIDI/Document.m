//
//  Document.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Document.h"
#import "MainWindowController.h"
#import "FileRepresentation.h"
#import "NAMidiRepresentation.h"

@interface Document ()
@property (strong, nonatomic) FileRepresentation *file;
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@end

@implementation Document

- (instancetype)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
    }
    return self;
}

- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError * _Nullable __autoreleasing *)outError
{
    _file = [[FileRepresentation alloc] initWithURL:url];
    _namidi = [[NAMidiRepresentation alloc] init];
    _namidi.file = _file;
    [_namidi parse];
    return YES;
}

- (void)makeWindowControllers
{
    MainWindowController *mainWC = [[MainWindowController alloc] init];
    mainWC.namidi = _namidi;
    [self addWindowController: mainWC];
}

#pragma mark Disable menu action

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)anItem
{
    if (@selector(saveDocument:) == anItem.action
        || @selector(saveDocumentAs:) == anItem.action
        || @selector(revertDocumentToSaved:) == anItem.action) {
        return NO;
    }
    else {
        return YES;
    }
}

@end
