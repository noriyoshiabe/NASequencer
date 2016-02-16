//
//  Document.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Document.h"
#import "MainWindowController.h"
#import "ApplicationController.h"

@interface Document ()
@property (strong, nonatomic) FileRepresentation *file;
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
    self.file = [[FileRepresentation alloc] initWithURL:url];
    return YES;
}

- (void)makeWindowControllers
{
    MainWindowController *mainWC = [[MainWindowController alloc] init];
    [self addWindowController: mainWC];
    
    [AppController showEditorWindowWithFile:self.file];
}

@end
